from pyfirmata import Arduino, util
board = Arduino('/dev/ttyUSB0')
fun handle_string(received):
	print('received:')
	println(received)
board.add_cmd_handler(STRING_DATA, handle_string())
println('sending')
board.send_sysex(0x71, util.str_to_two_byte_iter("maciek\0"))
while(True)
	board.iterate()
