# pi@pizero:/opt/arduino-control $ lsusb
# Bus 001 Device 004: ID 2341:0042 Arduino SA Mega 2560 R3 (CDC ACM)

import serial.tools.list_ports # for finding arduino

ARDUINO_VID = 0x2341
ARDUINO_PID = 0x0042

def find_arduino():
    ports = serial.tools.list_ports.comports()
    for port in ports:
        # Check if the device description contains "Arduino"
        if port.vid == ARDUINO_VID and port.pid == ARDUINO_PID:
            print('Arduino found on' + port.device)
            return port.device
    return None