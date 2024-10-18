from pyfirmata import Arduino, util, STRING_DATA
from bottle import get, post, run, template
import logging
import moskito_sub  
logging.basicConfig(level=logging.DEBUG)
logger = logging.getLogger(__name__)
# if __name__ == "__main__":
#     main()

lastReceived=[]
gate_ops = ['GO','GC','GS']
lighting_ops = ['BN','BF','SN','SF']
# BN bookshelf light on
# BF bookshelf light off
# SN staircase light on
# SF staircase light off
mqtt_ops = {
    'O': 'GO',
    'C': 'GC',
    'S': 'GS',
    '0' : 'BF',
    '1' : 'BN'
    # '' : 'SN',
    # '' : 'SF'
}

# the following corresponds to homebridge / mqttthing configuration
mqtt_gate_command_topic = 'gate/target/set'
mqtt_bookshelf_setstate_topic = 'bookshelf/state' # sends '0' or '1' as configured in homebridge
subscribe_to_topics = [mqtt_gate_command_topic,mqtt_bookshelf_setstate_topic]
def handle_string(*received):
    global lastReceived
    logger.debug('arduino responded via serial:')
    lastReceived.append("".join(map(chr, map(util.from_two_bytes,zip(received[0::2],received[1::2])))))
    if(len(lastReceived)>10):
      lastReceived.pop(0)
    logger.info(lastReceived)

# '/dev/ttyUSB0' on rpi
# /dev/tty.usbserial-14430 on Mac through dell dongle
# /dev/tty.usbserial-2230 on Mac through belkin station
# /dev/ttyACM0 is Iduino on pizero 
board = Arduino('/dev/ttyACM0')
board.add_cmd_handler(STRING_DATA, handle_string)

def send_string(msg):
    logger.debug('sending ' + msg + ' command to arduino via serial')
    board.send_sysex(0x71, util.str_to_two_byte_iter(msg+'\0'))

def on_incoming_mqtt_gate_cmd(topic, payload):
    try:
      send_string(mqtt_ops[payload])
    except Exception as inst:
      logger.error(type(inst))
      logger.error(inst.args)

it = util.Iterator(board)
it.start()

mosquitto_client = moskito_sub.start_client('127.0.0.1', subscribe_to_topics, on_incoming_mqtt_gate_cmd)
# mqtt_bookshelf_setstate_topic

@get('/last')
def index():
    global lastReceived
    return template('Last message: <b>{{lastMsg}}</b>!', lastMsg=lastReceived)


@post('/gate/<op>')
def gate(op):
    print("gate: "+ op)
    if(op in gate_ops):
        send_string(op)
    else:
        print('unknown op: '+op)
    return template('<b>Operation {{op}}</b>!', op=op)    


try:
    run(host='localhost', port=8080)
except Exception as exc:
    logger.error(type(inst))
    logger.error(inst.args)
    logger.error('exiting due to above exception, bye!')
    board.exit()
    mosquitto_client.loop_stop()


