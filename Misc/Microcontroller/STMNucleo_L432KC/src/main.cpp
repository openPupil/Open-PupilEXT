/*

This programm is used to produce a hardware trigger
for a Basler camera

Protocol:
Manually adjust trigger properties
Achtung Zeit ist in Mikrosekunden !
<T X Count_of_Trigger X Time_Trigger_on_us>
<TX10X1000000>

Beenden der PWM zwischendrin
<SX>

Baudrate: 115200
X is the seperator between the values

*/

/*------------------------Bibliotheken------------*/
#include "RawSerial.h"
#include "mbed.h"
#include <string>
#include <stdio.h>
#include <stdlib.h>

/*------------ Constant definitions --------------*/

#define BUFF_LENGTH 15
#define BAUDRATE 115200

#define TX_PIN USBTX
#define RX_PIN USBRX

#define LED_PIN_1 LED1
#define Trigger_Pin PB_0

RawSerial pc(TX_PIN, RX_PIN);
volatile char rx_buf[BUFF_LENGTH];
Timer timer_Stopper;

DigitalOut LED_Green(LED_PIN_1);
DigitalOut Trigger(Trigger_Pin);
Ticker myTick;

// Global Values
char M_or_T; // T = Manually Trigger Mode
uint32_t Manually_Values[2];
uint32_t Automatic_Values[1];

volatile int flag_1 = 0;
volatile int flag_2 = 0;
int LED_ticker = 0;
int Threshold_Ticker = 0;
int STOP = 0;

void onTick() {
  LED_Green = !LED_Green;
  Trigger = !Trigger;
  LED_ticker++;
  if (LED_ticker >= 2 * Threshold_Ticker) {
    myTick.detach();
    LED_Green = 0;
    Trigger = 0;
  }
  if (STOP == 1) {
    myTick.detach();
    LED_Green = 0;
    Trigger = 0;
  }
}

void serialCb() {

  char *pch;
  pch = strtok((char *)rx_buf, "X");

  M_or_T = *pch;

  if (M_or_T == 'T') {
    pch = strtok(NULL, "X");
    Manually_Values[0] = atoi(pch);

    pch = strtok(NULL, "X");
    Manually_Values[1] = atoi(pch);
  }

  switch (M_or_T) {

  case 'T':
    Threshold_Ticker = Manually_Values[0];
    STOP = 0;
    LED_ticker = 0;
    myTick.attach_us(&onTick, Manually_Values[1]);
    break;

  case 'S':
    STOP = 1;
    break;

  default:
    break;
  }
}

void callback() {
  if (pc.getc() == '<') {
    pc.putc('<');
    for (int i = 0; i < 20; i++) {

      rx_buf[i] = pc.getc();

      if (rx_buf[i] != '>') {
        pc.putc(rx_buf[i]);
      }

      if (rx_buf[i] == '>') {
        pc.putc('>');
        pc.putc('\n');
        flag_1 = 1;
        break;
      }
    }
  }
}

int main() {
  LED_Green = 0;
  LED_ticker = 0;
  Trigger = 0;
  pc.baud(115200);
  pc.attach(&callback, Serial::RxIrq);
  pc.printf("Program started! \n");

  while (1) {
    if (flag_1 == 1) {
      serialCb();
      flag_1 = 0;
    }
  }
}
