#include <RBD_Button.h>
#include <RBD_Timer.h>
#include <Firmata.h>
// hela room 4channel relay
const int relayPin = 22;      // helaA ch2 (orange)
const int relayPinB = 23;     // helaB ch3 (red)
const int relayPinC = 24;     // helaC ch4 (brown)
const int relayPin8ch1 = 26;  // worange (free)
const int relayPin8ch2 = 27;  // orange  (free)
const int relayPin8ch3 = 28;  // wgreen  (free)
const int relayPin8ch4 = 29;  // blue
const int relayPin8ch5 = 30;  // wblue     staircase upstairs bulb
const int relayPin8ch6 = 31;  // green
const int relayPin8ch7 = 32;  // wh-brown  staircase 'downstairs' bulb (billy white bookshelf lighting)
const int relayPin8ch8 = 33;  // brown     staircase middle bulb

// gate 4channel relay
const int relayPin4ch1 = 38;  // green                       Gate Open
const int relayPin4ch2 = 40;  // blue                        Gate Stop
const int relayPin4ch3 = 42;  // orange (instead of "red")   Gate Close
const int relayPin4ch4 = 44;  // brown                       Gate "One" - cyclic open, stop, close, stop, ...

const int gateRelaySignalDurationMillis = 1000;

struct RelayActuator {
  int pin;
  int ledState;
};

struct SwitchSensor {
  int pin;
  int lastSwitchState;
  int switchState;
  unsigned long lastDebounceTime;
  RelayActuator *relay;
};

// RBD::Button buttonTopRed(50);
// RBD::Button buttonTopGreen(48);
// RBD::Button buttonBottomRed(52??);
RelayActuator staircaseUpstairsBulb = { relayPin8ch5, LOW };
RelayActuator helaABulb = { relayPin, LOW };
RelayActuator helaBBulb = { relayPinB, LOW };
RelayActuator helaCBulb = { relayPinC, LOW };

RelayActuator gateOpenRelay = { relayPin4ch1, HIGH };
RelayActuator gateStopRelay = { relayPin4ch2, HIGH };
RelayActuator gateCloseRelay = { relayPin4ch3, HIGH };
// RelayActuator gateCyclicalRelay = { relayPin4ch4, LOW };

RelayActuator billyBookshelfLighting = { relayPin8ch7, LOW };

SwitchSensor staircaseTopGreen = { 48, HIGH, HIGH, 0, &staircaseUpstairsBulb };
SwitchSensor staircaseTopRed = { 50, HIGH, HIGH, 0, &staircaseUpstairsBulb };
SwitchSensor staircaseDownstairsRed = { 52, HIGH, HIGH, 0, &staircaseUpstairsBulb };
SwitchSensor staircaseDownstairsGreen = { 45, LOW, LOW, 0, &billyBookshelfLighting }; // TODO 47 or 45, to be confirmed

SwitchSensor helaSwitchSensorA = { 49, LOW, LOW, 0, &helaABulb };
SwitchSensor helaSwitchSensorB = { 51, LOW, LOW, 0, &helaBBulb };
SwitchSensor helaSwitchSensorC = { 53, LOW, LOW, 0, &helaCBulb };

const unsigned long debounceDelay = 50;
RBD::Timer staircaseTimerMiddle;
RBD::Timer staircaseTimerTop;
RBD::Timer staircaseTimerBottom;

RBD::Timer gateOpenTimer(gateRelaySignalDurationMillis);
RBD::Timer gateStopTimer(gateRelaySignalDurationMillis);
RBD::Timer gateCloseTimer(gateRelaySignalDurationMillis);

char switchLogMsg[56];
char preparePinLogMsg[29];

void prepareRelayActuator(int relayPinP, SwitchSensor switchSensor) {
  pinMode(relayPinP, OUTPUT);
  digitalWrite(relayPinP, switchSensor.lastSwitchState);
  snprintf(preparePinLogMsg, sizeof(preparePinLogMsg), "prepare relay pin %i to %i", relayPinP, switchSensor.lastSwitchState);
  Firmata.sendString(preparePinLogMsg);
}

void initRelayActuator(RelayActuator relayActuator) {
  pinMode(relayActuator.pin, OUTPUT);
  digitalWrite(relayActuator.pin, relayActuator.ledState);
}

void prepareWallSwitch(SwitchSensor switchSensor) {
  pinMode(switchSensor.pin, INPUT_PULLUP);
}

int processSwitchSensor(RelayActuator *relayActuator, SwitchSensor *switchSensor) {
  return process(relayActuator->pin, switchSensor->pin, &(switchSensor->lastSwitchState), &(switchSensor->lastDebounceTime), &(switchSensor->switchState), &(relayActuator->ledState));
}

void processSwitchSensorStaircase(RelayActuator *relayActuator, SwitchSensor *switchSensor) {
  int delta = processSwitchSensor(relayActuator, switchSensor);
  if (delta != 0) {
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
        ret = (*ledStateP) * 2 - 1;
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
  if (gateTimer->onExpired()) {
    // relay->ledState = HIGH;
    (*relay).ledState = HIGH;
    snprintf(switchLogMsg, sizeof(switchLogMsg), "timed out gate pin %i, setting to %i again", relay->pin, relay->ledState);
    Firmata.sendString(switchLogMsg);
  }
  digitalWrite(relay->pin, relay->ledState);
  digitalWrite(LED_BUILTIN, relay->ledState);
}

void setup() {
  Firmata.setFirmwareVersion(FIRMATA_FIRMWARE_MAJOR_VERSION, FIRMATA_FIRMWARE_MINOR_VERSION);
  Firmata.attach(STRING_DATA, onSerialCommandReceiveStringCallback);
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
  prepareRelayActuator(relayPin8ch7, staircaseDownstairsGreen);
  prepareRelayActuator(relayPin8ch8, staircaseDownstairsRed);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  staircaseTimerTop.setTimeout(250);
  staircaseTimerMiddle.setTimeout(500);
  staircaseTimerBottom.setTimeout(750);

  // prepare gate relay actuators
  initRelayActuator(gateOpenRelay);
  initRelayActuator(gateStopRelay);
  initRelayActuator(gateCloseRelay);
}

void loop() {
  processSwitchSensor(&helaABulb, &helaSwitchSensorA);
  processSwitchSensor(&helaBBulb, &helaSwitchSensorB);
  processSwitchSensor(&helaCBulb, &helaSwitchSensorC);
  processSwitchSensor(&billyBookshelfLighting, &staircaseDownstairsGreen);

  processSwitchSensorStaircase(&staircaseUpstairsBulb, &staircaseDownstairsRed);
  processSwitchSensorStaircase(&staircaseUpstairsBulb, &staircaseTopGreen);

  processGate(&gateOpenRelay, &gateOpenTimer);
  processGate(&gateCloseRelay, &gateCloseTimer);
  processGate(&gateStopRelay, &gateStopTimer);

  if (staircaseTimerMiddle.onRestart()) {
    // toggle middle light
    digitalWrite(relayPin8ch8, staircaseUpstairsBulb.ledState);
  }
  //  if(staircaseTimerBottom.onRestart()){
  //    // toggle bottom light
  //    digitalWrite(relayPin8ch7, staircaseUpstairsBulb.ledState);
  //  }
  //  if(staircaseTimerTop.onRestart()){
  //    // toggle top?bottom? light
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
void onSerialCommandReceiveStringCallback(char *myString) {
  if (strcmp(myString, "GO") == 0) {
    gateOpenRelay.ledState = LOW;
    gateOpenTimer.restart();
    Firmata.sendString("opening the gate");
  } else if (strcmp(myString, "GC") == 0) {
    gateCloseRelay.ledState = LOW;
    gateCloseTimer.restart();
    Firmata.sendString("closing the gate");
  } else if (strcmp(myString, "GS") == 0) {
    gateStopRelay.ledState = LOW;
    gateStopTimer.restart();
    Firmata.sendString("stopping the gate");
  }
}


void sysexCallback(byte command, byte argc, byte *argv) {
  Firmata.sendSysex(command, argc, argv);
}
