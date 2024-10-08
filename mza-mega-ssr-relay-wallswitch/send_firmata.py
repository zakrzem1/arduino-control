from pyfirmata import Arduino, util, STRING_DATA
from bottle import get, post, run, template
import logging
import moskito_sub  
logging.basicConfig(level=logging.DEBUG)

# if __name__ == "__main__":
#     main()

lastReceived='nothing'
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
    'true' : 'BN',
    'false' : 'BF',
    # '' : 'SN',
    # '' : 'SF'
}

# the following corresponds to homebridge / mqttthing configuration
mqtt_gate_command_topic = 'gate/target/set' 
mqtt_bookshelf_setstate_topic = 'bookshelf/state'
subscribe_to_topics = [mqtt_gate_command_topic,mqtt_bookshelf_setstate_topic]
def handle_string(*received):
    global lastReceived
    print('arduino responded via serial:')
    lastReceived="".join(map(chr, map(util.from_two_bytes,zip(received[0::2],received[1::2]))))
    print(lastReceived)

# '/dev/ttyUSB0' on rpi
# /dev/tty.usbserial-14430 on Mac through dell dongle
# /dev/tty.usbserial-2230 on Mac through belkin station
# /dev/ttyACM0 is Iduino on pizero 
board = Arduino('/dev/ttyACM0')
board.add_cmd_handler(STRING_DATA, handle_string)

def send_string(msg):
    print('sending ' + msg + ' command to arduino via serial')
    board.send_sysex(0x71, util.str_to_two_byte_iter(msg+'\0'))

def on_incoming_mqtt_gate_cmd(topic, payload):
    send_string(mqtt_ops[payload])

it = util.Iterator(board)
it.start()

moskito_sub.start_client('127.0.0.1', subscribe_to_topics, on_incoming_mqtt_gate_cmd)
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
except:
    board.exit()
    print('bye!')

