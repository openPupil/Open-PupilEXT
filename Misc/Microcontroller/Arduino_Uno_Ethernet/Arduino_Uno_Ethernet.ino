#include <TimerOne.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
// NOTE: This code uses the TimerOne library v1.1.1, which can be obtained from the Arduino IDE  
// library manager or from: https://github.com/PaulStoffregen/TimerOne/tree/master
// NOTE: This code uses the Ethernet library v2.0.2, which can be obtained from the Arduino IDE  
// library manager or from: https://github.com/arduino-libraries/Ethernet

#define SERIAL_BUFFER_SIZE 32 // 15 in Nucleo codes
#define SERIAL_BAUD_RATE 115200
#define SERIAL_READ_TIMEOUT_MS 500 

#define LED_PIN LED_BUILTIN
#define TRIGGER_PIN 9 

#define ETH_CS_PIN 10 // configure the CS pin
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; // You can provide an own mac address but it is not important
IPAddress ip(192, 168, 40, 200); // The IP is important, you can set your own if you wish
unsigned int listenPort = 7000; // Local port to listen on

// NOTE: Example serial command to the board: <TX30000X500000>
// This will set the time between rising and falling edges (= half period) to 500000 microseconds = 500 ms,
// and the number of rising edges (= number of full periods) until automatic termination to 30000 cycles,
// resulting in an image recording of 30000 frames in total, at 1 FPS 
// (which ultimately results in 30000/500=60 seconds of recording)  
// NOTE: The lowest possible FPS (in case of using this flashing code) is 0.00023283064 Hz

long int ReceivedValue1; // Use 64bit signed vars for overflow detection
long long int ReceivedValue2;
char SerialReadChar;
char SerialMessage[SERIAL_BUFFER_SIZE];
int SerialBytesReadOnce = 0;
int SerialBytesReadTotal = 0;
int SerialBytesAvailable = 0;
bool SerialReadFault = false;
unsigned long int TimeOfLastRead = 0;

char UDPMessage[UDP_TX_PACKET_MAX_SIZE];
int UDPPacketAvailable = 0;
EthernetUDP Udp;
bool UDPInUse = false;
bool UDPReadFault = false;

char* TokenBuffer;

int LEDTicksCount = 0;
long int LEDTicksThreshold = 0; // NOTE: We store the threshold itself (= twice the gotten value), not half of it (=the gotten value), like Nucleo codes do currently
unsigned long int DelayInterval = 0; 
bool TriggerRunning = false;
bool TriggerRunningLast = false;
bool RisingOrFalling = false;

void processReceivedMessage(char message[], bool readFault) {

  // Convert read char array
  TokenBuffer = strtok(message, "X");
  if(message[1] == 'T') {
    TokenBuffer = strtok(NULL, "X");
    ReceivedValue1 = atol(TokenBuffer);

    TokenBuffer = strtok(NULL, ">");
    ReceivedValue2 = atol(TokenBuffer);
  }

  // Detect possible overflow (instead of using unsigned 32bit, we use signed 64bit, because why not if it exists)
  if(readFault || ReceivedValue1 < 0 || ReceivedValue2 < 0) {
    // Serial.print("Camera triggering not running. \n");
    TriggerRunning = false;
    return;
  }
  // NOTE: Still, is it okay if we accept 0 values?

  if(message[1] == 'T') {
    LEDTicksThreshold = ReceivedValue1 *2;
    DelayInterval = (unsigned long int)ReceivedValue2;
    TriggerRunning = true;
    LEDTicksCount = 0;
  } else if(message[1] == 'S') {
    TriggerRunning = false;
  }
}

void checkMessageSerial() {

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
  processReceivedMessage(SerialMessage, SerialReadFault);
}

void checkMessageUDP() {
  UDPReadFault = (Udp.read(UDPMessage, UDP_TX_PACKET_MAX_SIZE) == 0);
  processReceivedMessage(UDPMessage, UDPReadFault);
}

void setup() {

  pinMode(LED_PIN, OUTPUT);
  pinMode(TRIGGER_PIN, OUTPUT);

  Serial.begin(SERIAL_BAUD_RATE);

  Ethernet.init(ETH_CS_PIN); // This number depends on the type of ethernet shield, but should be 10 for the basic original ethernet shield
  Ethernet.begin(mac, ip);

  if (!(Ethernet.hardwareStatus() == EthernetNoHardware) && !(Ethernet.linkStatus() == LinkOFF)) {
    Udp.begin(listenPort);
    UDPInUse = true;
  }
  
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
    LEDTicksCount++;
    RisingOrFalling = true;
  }

}

void loop() {

  SerialBytesAvailable = Serial.available();
  if(SerialBytesAvailable) {
    checkMessageSerial();
  }

  if(UDPInUse) {
    UDPPacketAvailable = Udp.parsePacket();
    if (UDPPacketAvailable) {
      checkMessageUDP();
    }
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
