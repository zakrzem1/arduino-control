from pyfirmata import Arduino, util, STRING_DATA
board = Arduino('/dev/ttyUSB0')
def handle_string(*received):
	print('received:') # TODO chunk in 2 bytes chunks
	print(util.from_two_bytes(received))
board.add_cmd_handler(STRING_DATA, handle_string)
print('sending')
board.send_sysex(0x71, util.str_to_two_byte_iter("maciek\0"))

while(True):
    while board.bytes_available():
        board.iterate()
