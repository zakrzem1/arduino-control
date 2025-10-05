
* installation
    * ensure you have python3 installed
    * install required python packages

    pip3 install paho-mqtt pyfirmata bottle

    * install MQTT broker
    
    sudo apt-get install mosquitto


* 2025-10-04 listing library versions on pizero

```
pi@pizero:/opt/arduino-control $ pip list
Package         Version
--------------- -------
Adafruit-GPIO   1.0.4
Adafruit-PureIO 1.1.7
asn1crypto      0.24.0
configparser    3.5.0b2
cryptography    2.6.1
entrypoints     0.3
enum34          1.1.6
ipaddress       1.0.17
keyring         17.1.1
keyrings.alt    3.1.1
paho-mqtt       1.5.1
pip             18.1
pycrypto        2.6.1
PyGObject       3.30.4
pyhocon         0.3.57
pyparsing       2.4.7
pyserial        3.4
pytz            2020.4
pyxdg           0.25
RPi.GPIO        0.7.0
SecretStorage   2.3.1
setuptools      40.8.0
six             1.12.0
spidev          3.5
wheel           0.32.3
```
```
pi@pizero:/opt/arduino-control $ lsusb
Bus 001 Device 004: ID 2341:0042 Arduino SA Mega 2560 R3 (CDC ACM)
Bus 001 Device 003: ID 0bda:8153 Realtek Semiconductor Corp. RTL8153 Gigabit Ethernet Adapter
Bus 001 Device 002: ID 05e3:0610 Genesys Logic, Inc. 4-port hub
Bus 001 Device 001: ID 1d6b:0002 Linux Foundation 2.0 root hub
```