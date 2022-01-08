#include <RBD_Button.h>
#include <RBD_Timer.h>
#include <Firmata.h>
const int relayPin     = 22; // helaA
const int relayPinB    = 23; // helaB
const int relayPinC    = 24; // helaC
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
RelayActuator helaABulb = {relayPin, LOW};
RelayActuator helaBBulb = {relayPinB, LOW};
RelayActuator helaCBulb = {relayPinC, LOW};

RelayActuator gateOpenRelay = {relayPin8ch1, LOW};
RelayActuator gateCloseRelay = {relayPin8ch2, LOW};
RelayActuator gateStopRelay = {relayPin8ch3, LOW};

SwitchSensor staircaseTopGreen = {48, HIGH, HIGH, 0, &staircaseUpstairsBulb};
SwitchSensor staircaseTopRed = {50, HIGH, HIGH, 0, &staircaseUpstairsBulb};
SwitchSensor staircaseDownstairsRed = {52, HIGH, HIGH, 0, &staircaseUpstairsBulb};
SwitchSensor helaSwitchSensorA = {49, HIGH, HIGH, 0, &helaABulb};
SwitchSensor helaSwitchSensorB = {51, HIGH, HIGH, 0, &helaBBulb};
SwitchSensor helaSwitchSensorC = {53, HIGH, HIGH, 0, &helaCBulb};

const unsigned long debounceDelay = 50;
RBD::Timer staircaseTimerMiddle;
RBD::Timer staircaseTimerTop;
RBD::Timer staircaseTimerBottom;
RBD::Timer gateOpenTimer;
RBD::Timer gateStopTimer;
RBD::Timer gateCloseTimer;
char switchLogMsg[56];
char preparePinLogMsg[29];

void prepareRelayActuator(int relayPinP, SwitchSensor switchSensor) {
  pinMode(relayPinP, OUTPUT);
  digitalWrite(relayPinP, switchSensor.lastSwitchState);
  snprintf(preparePinLogMsg, sizeof(preparePinLogMsg), "prepare relay pin %i to %i", relayPinP, switchSensor.lastSwitchState);
  Firmata.sendString(preparePinLogMsg);
}

void prepareWallSwitch(SwitchSensor switchSensor) {
  pinMode(switchSensor.pin, INPUT_PULLUP);
}

int processSwitchSensor(RelayActuator *relayActuator, SwitchSensor *switchSensor) {
  return process( relayActuator->pin, switchSensor->pin, &(switchSensor->lastSwitchState), &(switchSensor->lastDebounceTime), &(switchSensor->switchState), &(relayActuator->ledState));
}

void processSwitchSensorStaircase(RelayActuator *relayActuator, SwitchSensor *switchSensor) {
  int delta = processSwitchSensor(relayActuator, switchSensor);
  if (delta != 0 ) {
    staircaseTimerMiddle.restart();
    staircaseTimerBottom.restart();
  }
}

// return -1 when turned off, 0 if stay,  +1 when turned on
int process(int relayPinP, int switchSensorPinP, int *lastSwitchStateP, unsigned long *lastDebounceTimeP, int *switchStateP, int *ledStateP) {
  int ret = 0;
  int reading = digitalRead(switchSensorPinP);
  if (reading != *lastSwitchStateP) {
    *lastDebounceTimeP = millis();
  }
  if ((millis() - *lastDebounceTimeP) > debounceDelay) {
    if (reading != *switchStateP) {
      *switchStateP = reading;
      if (*switchStateP == LOW) {
        *ledStateP = !*ledStateP;
        ret =  (*ledStateP) * 2 - 1;
        snprintf(switchLogMsg, sizeof(switchLogMsg), "triggered switch %i to toggle relay pin %i set to %i", switchSensorPinP, relayPinP, *ledStateP);
        Firmata.sendString(switchLogMsg);
      }
    }
  }
  digitalWrite(relayPinP, *ledStateP);
  *lastSwitchStateP = reading;
  return ret;
}

void processGate(RelayActuator *relay, RBD::Timer *gateTimer) {
  if (relay->ledState == LOW) {
    digitalWrite(relay->pin, relay->ledState);
    digitalWrite(LED_BUILTIN, relay->ledState);

    relay->ledState = HIGH;
    gateTimer->restart();
  }
  if (gateTimer->onRestart()) {
    // toggle gate open relay
    digitalWrite(relay->pin, HIGH);
    digitalWrite(LED_BUILTIN, HIGH);
  }
}

void setup() {
  Firmata.setFirmwareVersion(FIRMATA_FIRMWARE_MAJOR_VERSION, FIRMATA_FIRMWARE_MINOR_VERSION);
  Firmata.attach(STRING_DATA, stringCallback);
  Firmata.attach(START_SYSEX, sysexCallback);
  Firmata.begin(57600);

  Firmata.sendString("--- Starting mza mega ssr relay wallswitch ---");

  prepareWallSwitch(staircaseDownstairsRed);
  prepareWallSwitch(staircaseTopGreen);
  prepareWallSwitch(helaSwitchSensorA);
  prepareWallSwitch(helaSwitchSensorB);
  prepareWallSwitch(helaSwitchSensorC);

  prepareRelayActuator(relayPin, helaSwitchSensorA);
  prepareRelayActuator(relayPinB, helaSwitchSensorB);
  prepareRelayActuator(relayPinC, helaSwitchSensorC);

  prepareRelayActuator(relayPin8ch5, staircaseDownstairsRed);
  prepareRelayActuator(relayPin8ch7, staircaseDownstairsRed);
  prepareRelayActuator(relayPin8ch8, staircaseDownstairsRed);

  pinMode(LED_BUILTIN, OUTPUT);

  staircaseTimerTop.setTimeout(250);
  staircaseTimerMiddle.setTimeout(500);
  staircaseTimerBottom.setTimeout(750);
  gateOpenTimer.setTimeout(500);
  gateCloseTimer.setTimeout(500);
  gateStopTimer.setTimeout(500);
}

void loop() {
  processSwitchSensor(&helaABulb, &helaSwitchSensorA);
  processSwitchSensor(&helaBBulb, &helaSwitchSensorB);
  processSwitchSensor(&helaCBulb, &helaSwitchSensorC);

  processSwitchSensorStaircase(&staircaseUpstairsBulb, &staircaseDownstairsRed);
  processSwitchSensorStaircase(&staircaseUpstairsBulb, &staircaseTopGreen);

  processGate(&gateOpenRelay, &gateOpenTimer);
  processGate(&gateCloseRelay, &gateCloseTimer);
  processGate(&gateStopRelay, &gateStopTimer);

  if (staircaseTimerMiddle.onRestart()) {
    // toggle middle light
    digitalWrite(relayPin8ch8, staircaseUpstairsBulb.ledState);
  }
  if (staircaseTimerBottom.onRestart()) {
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
  if (strcmp(myString, "GO") == 0) {
    gateOpenRelay.ledState = LOW;
  } else if (strcmp(myString, "GC") == 0) {
    gateCloseRelay.ledState = LOW;
  } else if (strcmp(myString, "GS") == 0) {
    gateStopRelay.ledState = LOW;
  }
  Firmata.sendString(myString);
}


void sysexCallback(byte command, byte argc, byte *argv)
{
  Firmata.sendSysex(command, argc, argv);
}
