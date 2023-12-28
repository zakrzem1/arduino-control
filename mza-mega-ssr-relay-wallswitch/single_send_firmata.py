from pyfirmata import Arduino, util, STRING_DATA
#from bottle import get, post, run, template
import sys
import logging
import moskito_sub  
logging.basicConfig(level=logging.DEBUG)

# if __name__ == "__main__":
#     main()

lastReceived='nothing'
gate_ops = ['GO','GC','GS','GY']
mqtt_gate_ops = {
    'O': 'GO',
    'C': 'GC',
    'S': 'GS',
    'Y': 'GY'
}
mqtt_gate_command_topic = 'gate/target/set'

def handle_string(*received):
    global lastReceived
    print('received:')
    lastReceived="".join(map(chr, map(util.from_two_bytes,zip(received[0::2],received[1::2]))))
    print(lastReceived)

# '/dev/ttyUSB0' on rpi
# /dev/tty.usbserial-14430 on Mac through dell dongle
# /dev/tty.usbserial-2230 on Mac through belkin station
# /dev/ttyACM0 is Iduiio on pizero 
board = Arduino('/dev/ttyACM0')
board.add_cmd_handler(STRING_DATA, handle_string)

def send_string(msg):
    print('sending')
    board.send_sysex(0x71, util.str_to_two_byte_iter(msg+'\0'))

def on_incoming_mqtt_gate_cmd(topic, payload):
    send_string(mqtt_gate_ops[payload])

it = util.Iterator(board)
it.start()

moskito_sub.start_client('127.0.0.1', mqtt_gate_command_topic, on_incoming_mqtt_gate_cmd)

def index():
    global lastReceived
    return template('Last message: <b>{{lastMsg}}</b>!', lastMsg=lastReceived)


def gate(op):
    print("gate: "+ op)
    if(op in gate_ops):
        send_string(op)
    else:
        print('unknown op: '+op)

gate(sys.argv[1])

