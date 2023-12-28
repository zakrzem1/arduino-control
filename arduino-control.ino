const int relayPin =  22;
const int relayPinB =  23;
const int relayPinC =  24;
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

void prepare(int relayPinP, int switchSensorPinP, int lastSwitchStateP){
  pinMode(relayPinP, OUTPUT);
  pinMode(switchSensorPinP, INPUT_PULLUP);
  digitalWrite(relayPinP, lastSwitchStateP);
  Serial.print("prepare relay pin ");
  Serial.print(relayPinP);
  Serial.print(" to ");
  Serial.println(lastSwitchStateP);
}

void process(int relayPinP, int switchSensorPinP, int *lastSwitchStateP, unsigned long *lastDebounceTimeP, int *switchStateP, int *ledStateP){
  int reading = digitalRead(switchSensorPinP);
  if (reading != *lastSwitchStateP) {
    *lastDebounceTimeP = millis();
  }
  if((millis()-*lastDebounceTimeP)> debounceDelay){
    if (reading != *switchStateP) {
      *switchStateP = reading;
      if (*switchStateP == LOW){
        *ledStateP = !*ledStateP;
        Serial.print("process relay pin ");
        Serial.print(relayPinP);
        Serial.print(" to ");
        Serial.println(*lastSwitchStateP);
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
  pinMode(ledPin, OUTPUT);
}

void loop() {
  process(relayPin, switchSensorPin, &lastSwitchState, &lastDebounceTime, &switchState, &ledState);
  process(relayPinB, switchSensorPinB, &lastSwitchStateB, &lastDebounceTimeB, &switchStateB, &ledStateB);
  process(relayPinC, switchSensorPinC, &lastSwitchStateC, &lastDebounceTimeC, &switchStateC, &ledStateC);
  digitalWrite(ledPin, !(ledState && ledStateB && ledStateC));
}
