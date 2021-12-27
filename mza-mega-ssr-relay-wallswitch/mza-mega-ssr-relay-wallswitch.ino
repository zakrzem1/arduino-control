#include <RBD_Button.h>
#include <RBD_Timer.h>
#include <Firmata.h>
const int relayPin =  22;    // wardrobe
const int relayPinB =  23;   // wardrobe
const int relayPinC =  24;   // wardrobe
const int relayPin8ch1 = 26; // worange
const int relayPin8ch2 = 27; // orange
const int relayPin8ch3 = 28; // wgreen
const int relayPin8ch4 = 29; // blue
const int relayPin8ch5 = 30; // wblue     staircase upstairs bulb
const int relayPin8ch6 = 31; // green
const int relayPin8ch7 = 32; // wh-brown  staircase downstairs bulb
const int relayPin8ch8 = 33; // brown     staircase middle bulb

struct RelayActuator
{
    int pin;
    int ledState;
};
struct SwitchSensor
{
    int pin;
    int lastSwitchState;
    int switchState;
    unsigned long lastDebounceTime;
    RelayActuator *relay;
};

// RBD::Button buttonTopRed(50);
// RBD::Button buttonTopGreen(48);
// RBD::Button buttonBottomRed(52??);
RelayActuator staircaseUpstairsBulb = {relayPin8ch5, LOW};
SwitchSensor staircaseTopGreen = {48, HIGH, HIGH, 0, &staircaseUpstairsBulb};
SwitchSensor staircaseTopRed = {50, HIGH, HIGH, 0, &staircaseUpstairsBulb};
SwitchSensor staircaseDownstairsRed = {52, HIGH, HIGH, 0, &staircaseUpstairsBulb};
const int switchSensorPin = 49;
const int switchSensorPinB = 51;
const int switchSensorPinC = 53;
int lastSwitchState = HIGH;
int lastSwitchStateB = HIGH;
int lastSwitchStateC = HIGH;
int switchState = HIGH;
int switchStateB = HIGH;
int switchStateC = HIGH;
int ledState = HIGH;
int ledStateB = HIGH;
int ledStateC = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long lastDebounceTimeB = 0;
unsigned long lastDebounceTimeC = 0;
const unsigned long debounceDelay = 50;
RBD::Timer staircaseTimerMiddle;
RBD::Timer staircaseTimerTop;
RBD::Timer staircaseTimerBottom;
char switchLogMsg[56];
char preparePinLogMsg[29];

void prepare(int relayPinP, int switchSensorPinP, int lastSwitchStateP){
  pinMode(relayPinP, OUTPUT);
  pinMode(switchSensorPinP, INPUT_PULLUP);
  digitalWrite(relayPinP, lastSwitchStateP);
  snprintf(preparePinLogMsg, sizeof(preparePinLogMsg), "prepare relay pin %i to %i",relayPinP, lastSwitchStateP);
  Firmata.sendString(preparePinLogMsg);
}

void prepareRelayActuator(int relayPinP, SwitchSensor switchSensor){
  pinMode(relayPinP, OUTPUT);
}

void prepareWallSwitch(SwitchSensor switchSensor){
  pinMode(switchSensor.pin, INPUT_PULLUP);
}

void processSwitchSensor(RelayActuator *relayActuator, SwitchSensor *switchSensor){
  int delta = process( relayActuator->pin, switchSensor->pin, &(switchSensor->lastSwitchState), &(switchSensor->lastDebounceTime), &(switchSensor->switchState), &(relayActuator->ledState));
  if (delta != 0 ){
    staircaseTimerMiddle.restart();
    staircaseTimerBottom.restart();
  }
}

// return -1 when turned off, 0 if stay,  +1 when turned on
int process(int relayPinP, int switchSensorPinP, int *lastSwitchStateP, unsigned long *lastDebounceTimeP, int *switchStateP, int *ledStateP){
  int ret = 0;
  int reading = digitalRead(switchSensorPinP);
  if (reading != *lastSwitchStateP) {
    *lastDebounceTimeP = millis();
  }
  if((millis()-*lastDebounceTimeP)> debounceDelay){
    if (reading != *switchStateP) {
      *switchStateP = reading;
      if (*switchStateP == LOW){
        *ledStateP = !*ledStateP;
        ret =  (*ledStateP) * 2 - 1;
        snprintf(switchLogMsg, sizeof(switchLogMsg), "triggered switch %i to toggle relay pin %i set to %i",switchSensorPinP, relayPinP, *ledStateP);
        Firmata.sendString(switchLogMsg);
      }
    }
  }
  digitalWrite(relayPinP, *ledStateP);
  *lastSwitchStateP = reading;
  return ret;
}

void setup() {
  Firmata.setFirmwareVersion(FIRMATA_FIRMWARE_MAJOR_VERSION, FIRMATA_FIRMWARE_MINOR_VERSION);
  Firmata.attach(STRING_DATA, stringCallback);
  Firmata.attach(START_SYSEX, sysexCallback);
  Firmata.begin(57600);

  Firmata.sendString("--- Start Serial Monitor for mza mega ssr relay wallswitch ---");
  prepare(relayPin, switchSensorPin, lastSwitchState);
  prepare(relayPinB, switchSensorPinB, lastSwitchStateB);
  prepare(relayPinC, switchSensorPinC, lastSwitchStateC);  

  prepareWallSwitch(staircaseDownstairsRed);
  prepareWallSwitch(staircaseTopGreen);
  
  prepareRelayActuator(relayPin8ch5, staircaseDownstairsRed);
  prepareRelayActuator(relayPin8ch7, staircaseDownstairsRed);
  prepareRelayActuator(relayPin8ch8, staircaseDownstairsRed);
  
  pinMode(LED_BUILTIN, OUTPUT);
  
  staircaseTimerMiddle.setTimeout(500);
  staircaseTimerTop.setTimeout(250);
  staircaseTimerBottom.setTimeout(750);
}

void loop() {
  process(relayPin, switchSensorPin, &lastSwitchState, &lastDebounceTime, &switchState, &ledState);
  process(relayPinB, switchSensorPinB, &lastSwitchStateB, &lastDebounceTimeB, &switchStateB, &ledStateB);
  process(relayPinC, switchSensorPinC, &lastSwitchStateC, &lastDebounceTimeC, &switchStateC, &ledStateC);

  processSwitchSensor(&staircaseUpstairsBulb, &staircaseDownstairsRed);
  processSwitchSensor(&staircaseUpstairsBulb, &staircaseTopGreen);

  if(staircaseTimerMiddle.onRestart()){
    // toggle middle light
    digitalWrite(relayPin8ch8, staircaseUpstairsBulb.ledState);
  }
  if(staircaseTimerBottom.onRestart()){
    // toggle bottom light
    digitalWrite(relayPin8ch7, staircaseUpstairsBulb.ledState);
  }
//  if(staircaseTimerTop.onRestart()){
//    // toggle bottom light
//    digitalWrite(relayPin8ch5, staircaseTopRed->ledState);
//  }
//  if(buttonTopRed.onPressed()) {
//    staircaseTimerMidlle.restart();
//    staircaseTimerBottom.restart();
//    Serial.println("Button Pressed");
//  }
  while (Firmata.available()) {
    Firmata.processInput();
  }
}

// firmata callbacks
void stringCallback(char *myString)
{
  if(strcmp(myString, "GO") == 0){
    digitalWrite(LED_BUILTIN, HIGH);
  }else if (strcmp(myString, "GC")==0){
    digitalWrite(LED_BUILTIN, LOW);
  }
  Firmata.sendString(myString);  
}


void sysexCallback(byte command, byte argc, byte *argv)
{
  Firmata.sendSysex(command, argc, argv);
}
