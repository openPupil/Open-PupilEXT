#include <TimerOne.h>
// NOTE: This code uses the TimerOne library v1.1.1, which can be obtained from the Arduino IDE  
// library manager or from: https://github.com/PaulStoffregen/TimerOne/tree/master

#define SERIAL_BUFFER_SIZE 32 // 15 in Nucleo codes
#define SERIAL_BAUD_RATE 115200
#define SERIAL_READ_TIMEOUT_MS 500 

#define LED_PIN LED_BUILTIN
#define TRIGGER_PIN 9 

// NOTE: Example serial command to the board: <TX30000X500000>
// This will set the time between rising and falling edges (= half period) to 500000 microseconds = 500 ms,
// and the number of rising edges (= number of full periods) until automatic termination to 30000 cycles,
// resulting in an image recording of 30000 frames in total, at 1 FPS 
// (which ultimately results in 30000/500=60 seconds of recording)  
// NOTE: The lowest possible FPS (in case of using this flashing code) is 0.00023283064 Hz
// Even in case of 1000 FPS, the tick counter should last 4294967295/1000/60 =~ 71582 minutes safely

long long int ReceivedValue1;
long long int ReceivedValue2;
char SerialReadChar;
char SerialMessage[SERIAL_BUFFER_SIZE];
int SerialBytesReadOnce = 0;
int SerialBytesReadTotal = 0;
int SerialBytesAvailable = 0;
bool SerialReadFault = false;
unsigned long int TimeOfLastRead = 0;
char* TokenBuffer;

unsigned long int LEDTicksCount = 0;
unsigned long int LEDTicksThreshold = 0;
unsigned long int DelayInterval = 0; 
bool TriggerRunning = false; // true;
bool TriggerRunningLast = false; // true;
bool RisingOrFalling = false;

void checkMessage() {

  // Read data from serial
  SerialBytesReadTotal = 0;
  SerialReadFault = false;
  TimeOfLastRead = millis();
  while((millis() - TimeOfLastRead) < SERIAL_READ_TIMEOUT_MS) {
    SerialBytesReadOnce = 0;
    SerialReadChar = -1;
    while(SerialBytesReadOnce < SerialBytesAvailable) {
      SerialReadChar = Serial.read();
      if(SerialBytesReadTotal < SERIAL_BUFFER_SIZE && SerialReadChar >= 0) {
        SerialMessage[SerialBytesReadTotal] = SerialReadChar;
        TimeOfLastRead = millis();
      } else {
        SerialReadFault = true;
      }
      SerialBytesReadOnce++;
      SerialBytesReadTotal++;
      SerialBytesAvailable = Serial.available();
    }

    // Wait until timeout
    while(!SerialBytesAvailable && (millis() - TimeOfLastRead) < SERIAL_READ_TIMEOUT_MS) {
      SerialBytesAvailable = Serial.available();
    }
  }

  // Convert read char array
  TokenBuffer = strtok(SerialMessage, "X");
  if(SerialMessage[1] == 'T') {
    TokenBuffer = strtok(NULL, "X");
    ReceivedValue1 = atoll(TokenBuffer);

    TokenBuffer = strtok(NULL, ">");
    ReceivedValue2 = atoll(TokenBuffer);
  }

  // Detect possible overflow
  if(SerialReadFault || ReceivedValue1 < 0 || ReceivedValue2 < 0) {
    //Serial.print("Camera triggering not running. \n");
    TriggerRunning = false;
    return;
  }
  // NOTE: Still, is it okay if we accept 0 values?

  if(SerialMessage[1] == 'T') {
    LEDTicksThreshold = (unsigned long int)ReceivedValue1;
    DelayInterval = (unsigned long int)ReceivedValue2;
    TriggerRunning = true;
    LEDTicksCount = 0;
  } else if(SerialMessage[1] == 'S') {
    TriggerRunning = false;
  }

}

void setup() {

  pinMode(LED_PIN, OUTPUT);
  pinMode(TRIGGER_PIN, OUTPUT);

  Serial.begin(SERIAL_BAUD_RATE);
  
  digitalWrite(LED_PIN, LOW);
  digitalWrite(TRIGGER_PIN, LOW);
  LEDTicksCount = 0;
  
  Serial.print("Program started! \n");

}

void makeTick() {

  if(RisingOrFalling) {
    digitalWrite(TRIGGER_PIN, HIGH);
    digitalWrite(LED_PIN, HIGH);
    LEDTicksCount++;
    RisingOrFalling = false;
  } else {
    digitalWrite(TRIGGER_PIN, LOW);
    digitalWrite(LED_PIN, LOW);
    //LEDTicksCount++;
    RisingOrFalling = true;
  }

}

void loop() {

  SerialBytesAvailable = Serial.available();

  if(SerialBytesAvailable) {
    checkMessage();
  }

  if(TriggerRunning && LEDTicksThreshold != 0 && LEDTicksCount >= LEDTicksThreshold) {
    TriggerRunning = false;
    Serial.print("Camera triggering stopped. \n");
  }

  if(TriggerRunning && !TriggerRunningLast) {
    Serial.print("Starting camera triggering. \n");
    // Serial.print("Half period in microseconds: " + String(DelayInterval) + "\n");
    Timer1.initialize(DelayInterval); // DelayInterval is the time of a HALF period
    Timer1.attachInterrupt(makeTick);
  }

  if(!TriggerRunning && TriggerRunningLast) {
    Timer1.detachInterrupt();
  }

  TriggerRunningLast = TriggerRunning;
  
}

// Arduino Uno flashing code 
// The provided code for the Arduino Uno was written by Gábor Bényei, licensed under MIT License
// See the lincense file in this repostory:
// Microcontroller/Arduino_Uno/LICENSE
