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
int once=4;

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
  int switchSensorPinP = switchSensor.pin;
  int lastSwitchStateP = switchSensor.lastSwitchState;
  pinMode(relayPinP, OUTPUT);
  pinMode(switchSensorPinP, INPUT_PULLUP);
  digitalWrite(relayPinP, lastSwitchStateP);
  Serial.print(" prepare sensor switch pin ");
  Serial.print(switchSensorPinP);
  Serial.print(" for relay pin ");
  Serial.print(relayPinP);
  Serial.print(" set initial relay state to ");
  Serial.println(lastSwitchStateP);
  Serial.print(" current switch sensor status=");
  Serial.println(digitalRead(switchSensorPinP));
}

void processSwitchSensor(int relayPinP, SwitchSensor *switchSensor){
  if(once>0){
    if(once % 4 == 0){
      Serial.println("processSwitchSensor ");
      Serial.println(switchSensor->pin);
      Serial.println(switchSensor->lastSwitchState);
      Serial.println(switchSensor->lastDebounceTime);
      Serial.println(switchSensor->switchState);
      Serial.println(switchSensor->ledState);
      Serial.println();
    }
    once--;
  }
  process( relayPinP, switchSensor->pin, &(switchSensor->lastSwitchState), &(switchSensor->lastDebounceTime), &(switchSensor->switchState), &(switchSensor->ledState));
  
}
void process(int relayPinP, int switchSensorPinP, int *lastSwitchStateP, unsigned long *lastDebounceTimeP, int *switchStateP, int *ledStateP){
  int reading = digitalRead(switchSensorPinP);
  if (reading != *lastSwitchStateP) {
    *lastDebounceTimeP = millis();
  }
  if((millis()-*lastDebounceTimeP)> debounceDelay){
    if (reading != *switchStateP) {
      *switchStateP = reading;
      if(switchSensorPinP == 52){
        Serial.print("switchStateP set to");
        Serial.print(*switchStateP );
        Serial.println(millis());
      }
      if (*switchStateP == LOW){
        *ledStateP = !*ledStateP;
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
}

void setup() {
  Serial.begin(9600);  
  Serial.println("--- Start Serial Monitor for mza mega ssr relay wallswitch ---");
  prepare(relayPin, switchSensorPin, lastSwitchState);
  prepare(relayPinB, switchSensorPinB, lastSwitchStateB);
  prepare(relayPinC, switchSensorPinC, lastSwitchStateC);  

  prepareSwitchSensor( relayPin8ch5, staircaseTopRed); 
  prepareSwitchSensor( relayPin8ch7, staircaseTopGreen); 
//  prepare(relayPin8ch5, switchSensorPinStaircaseTopRed, lastSwitchState);  
//  prepare(relayPin8ch7, switchSensorPinStaircaseTopRed, lastSwitchState);  
//  prepare(relayPin8ch8 , switchSensorPinStaircaseTopRed, lastSwitchState);  
  pinMode(ledPin, OUTPUT);
}

void loop() {
  process(relayPin, switchSensorPin, &lastSwitchState, &lastDebounceTime, &switchState, &ledState);
  process(relayPinB, switchSensorPinB, &lastSwitchStateB, &lastDebounceTimeB, &switchStateB, &ledStateB);
  process(relayPinC, switchSensorPinC, &lastSwitchStateC, &lastDebounceTimeC, &switchStateC, &ledStateC);

  processSwitchSensor(relayPin8ch5, &staircaseTopRed);
  processSwitchSensor(relayPin8ch7, &staircaseTopGreen);
  //digitalWrite(ledPin, !(ledState && ledStateB && ledStateC && (&staircaseTopRed)->ledState && (&staircaseTopGreen)->ledState));
  digitalWrite(ledPin, !(&staircaseTopRed)->ledState); //&& (&staircaseTopGreen)->ledState)
}
