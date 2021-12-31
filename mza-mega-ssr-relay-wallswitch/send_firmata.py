from pyfirmata import Arduino, util, STRING_DATA
from bottle import get, post, run, template
import logging
import moskito_sub  
logging.basicConfig(level=logging.DEBUG)

# if __name__ == "__main__":
#     main()

lastReceived='nothing'
gate_ops = ('GO','GC','GS')
mqtt_gate_ops = ('open','close','stop')
mqtt_gate_ops = {
    'open': 'GO',
    'close': 'GC',
    'stop': 'GS'
}
def handle_string(*received):
    global lastReceived
    print('received:')
    lastReceived="".join(map(chr, map(util.from_two_bytes,zip(received[0::2],received[1::2]))))
    print(lastReceived)

# '/dev/ttyUSB0' on rpi
# /dev/tty.usbserial-14430 on Mac through dell dongle
# /dev/tty.usbserial-2230 on Mac through belkin station
board = Arduino('/dev/tty.usbserial-14430')
board.add_cmd_handler(STRING_DATA, handle_string)

def send_string(msg):
    print('sending')
    board.send_sysex(0x71, util.str_to_two_byte_iter(msg+'\0'))

def on_incoming_mqtt_gate_cmd(op):
    send_string(mqtt_gate_ops[op])

it = util.Iterator(board)
it.start()

moskito_sub.start_client('192.168.1.36', 'gate', on_incoming_mqtt_gate_cmd)

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

