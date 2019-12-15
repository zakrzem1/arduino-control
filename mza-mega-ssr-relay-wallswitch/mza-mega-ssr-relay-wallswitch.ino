#include <RBD_Button.h>
#include <RBD_Timer.h>

const int relayPin =  22; // garderoba
const int relayPinB =  23;
const int relayPinC =  24;
const int relayPin8ch1 = 26; // bpom
const int relayPin8ch2 = 27; // pom
const int relayPin8ch3 = 28; // bziel
const int relayPin8ch4 = 29; // nieb
const int relayPin8ch5 = 30; // bnieb 
const int relayPin8ch6 = 31; // ziel
const int relayPin8ch7 = 32; // wh-brown
const int relayPin8ch8 = 33; // brown

struct SwitchSensor
{
    int pin;
    int lastSwitchState;
    int switchState;
    unsigned long lastDebounceTime;
    int ledState;
};
// RBD::Button buttonTopRed(50);
// RBD::Button buttonTopGreen(52);
// RBD::Button buttonBottomRed(??);

SwitchSensor staircaseTopRed = {50, HIGH, HIGH, 0, HIGH};
SwitchSensor staircaseTopGreen = {52, HIGH, HIGH, 0, HIGH};
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

void prepareSwitchSensor(int relayPinP, SwitchSensor switchSensor){
  pinMode(relayPinP, OUTPUT);
  digitalWrite(relayPinP, switchSensor.lastSwitchState);
  pinMode(switchSensor.pin, INPUT_PULLUP);
  // Serial.print(" prepare sensor switch pin ");
  // Serial.print(switchSensor.pin);
  // Serial.print(" for relay pin ");
  // Serial.print(relayPinP);
  // Serial.print(" set initial relay state to ");
  // Serial.println(switchSensor.lastSwitchState);
  // Serial.print(" current switch sensor status=");
  // Serial.println(digitalRead(switchSensor.pin));
}

void processSwitchSensor(int relayPinP, SwitchSensor *switchSensor){
  int delta = process( relayPinP, switchSensor->pin, &(switchSensor->lastSwitchState), &(switchSensor->lastDebounceTime), &(switchSensor->switchState), &(switchSensor->ledState));
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

  prepareSwitchSensor( relayPin8ch5, staircaseTopRed); 
  prepareSwitchSensor( relayPin8ch7, staircaseTopRed); // staircaseTopGreen
  prepareSwitchSensor( relayPin8ch8, staircaseTopRed); // staircase 
  
  pinMode(ledPin, OUTPUT);
  
  staircaseTimerMiddle.setTimeout(1000);
  staircaseTimerTop.setTimeout(2000);
  staircaseTimerBottom.setTimeout(2000);
}

void loop() {
  process(relayPin, switchSensorPin, &lastSwitchState, &lastDebounceTime, &switchState, &ledState);
  process(relayPinB, switchSensorPinB, &lastSwitchStateB, &lastDebounceTimeB, &switchStateB, &ledStateB);
  process(relayPinC, switchSensorPinC, &lastSwitchStateC, &lastDebounceTimeC, &switchStateC, &ledStateC);

  processSwitchSensor(relayPin8ch5, &staircaseTopRed);
  //processSwitchSensor(relayPin8ch7, &staircaseTopGreen);
  //digitalWrite(ledPin, !(ledState && ledStateB && ledStateC && (&staircaseTopRed)->ledState && (&staircaseTopGreen)->ledState));
  digitalWrite(ledPin, !(&staircaseTopRed)->ledState); //&& (&staircaseTopGreen)->ledState)

  
  if(staircaseTimerMiddle.onRestart()){
    // toggle middle light
    digitalWrite(relayPin8ch8, staircaseTopRed.ledState);
  }
  if(staircaseTimerBottom.onRestart()){
    // toggle bottom light
    digitalWrite(relayPin8ch7, staircaseTopRed.ledState);
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
