#include <RBD_Button.h>
#include <RBD_Timer.h>

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
const int ledPin = 13;
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

void prepare(int relayPinP, int switchSensorPinP, int lastSwitchStateP){
  pinMode(relayPinP, OUTPUT);
  pinMode(switchSensorPinP, INPUT_PULLUP);
  digitalWrite(relayPinP, lastSwitchStateP);
  Serial.print("prepare relay pin ");
  Serial.print(relayPinP);
  Serial.print(" to ");
  Serial.println(lastSwitchStateP);
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
        Serial.print("triggered switch");
        Serial.print(switchSensorPinP);
        Serial.print(" to toggle relay pin ");
        Serial.print(relayPinP);
        Serial.print(" set to ");
        Serial.println(*ledStateP);
      }
    }
  }
  digitalWrite(relayPinP, *ledStateP);
  *lastSwitchStateP = reading;
  return ret;
}

void setup() {
  Serial.begin(9600);  
  Serial.println("--- Start Serial Monitor for mza mega ssr relay wallswitch ---");
  prepare(relayPin, switchSensorPin, lastSwitchState);
  prepare(relayPinB, switchSensorPinB, lastSwitchStateB);
  prepare(relayPinC, switchSensorPinC, lastSwitchStateC);  

  prepareWallSwitch(staircaseDownstairsRed);
  prepareWallSwitch(staircaseTopGreen);
  
  prepareRelayActuator(relayPin8ch5, staircaseDownstairsRed);
  prepareRelayActuator(relayPin8ch7, staircaseDownstairsRed);
  prepareRelayActuator(relayPin8ch8, staircaseDownstairsRed);
  
  pinMode(ledPin, OUTPUT);
  
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
}
