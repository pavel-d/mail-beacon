#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>
#include <avr/sleep.h>
#include <OneButton.h>
#include <EEPROM.h>
#include "Beacon.h"
#include "Blink.h"
#include "Packet.h"

#define SyncWord 0xF3
#define TxPower 20
#define SignalBandwidth 125E3
#define SpreadingFactor 12
#define BAND 433E6  

#define NSS 10
#define MOSI 14
#define MISO 15
#define SCK 16

#define NRESET 1
#define DI01 2

#define BTN1 11
#define PA4 0
#define PA7 3
#define LED 4
#define VSENS 5

#define BEACON_ID_ADDRESS 0x00

long wokeUpAt;
bool shouldTransmit;
uint8_t beaconID[PACKET_BEACON_ID_SIZE];
enum BeaconMode { TRANSMIT = 0, RESET = 2};

// TRANSMIT is default
BeaconMode beaconMode = TRANSMIT;

// Calculated using linear regression
// Due to chosen resistor dividors values and min voltage reg value, the actual range
// is between 3300 and 4800 millivolts
float voltageMultiplier = 4813.6192;
float voltageIntercept = -10.7395;

void sleep() {
  Serial.flush();
  LoRa.sleep();
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  sleep_cpu();
}


void onClick() {
  shouldTransmit = true;
  sleep_disable();
}

void onLongPress() {
  beaconMode = RESET;
}

OneButton button(BTN1);
Beacon beacon;

void handleButtonInterrupt() {
  wokeUpAt = millis();
  button.tick();
}

void handleExternalTrigger() {
  shouldTransmit = true;
  sleep_disable();
}

void initBeaconID(bool force) {
  if (!force) {
    EEPROM.get(BEACON_ID_ADDRESS, beaconID);

    for (int i = 0; i < PACKET_BEACON_ID_SIZE; i++) {
      if (beaconID[i] != 0) {
        return; // Address has been previously initialized, no need to re-assign
      }
    }
  }

  // Generate a random ID and save to EEPROM
  randomSeed(analogRead(PIN_PB0));

  for (int i = 0; i < PACKET_BEACON_ID_SIZE; i++) {
    beaconID[i] = random(256);
  }

  EEPROM.put(BEACON_ID_ADDRESS, beaconID);
}

void setup() {
  initBeaconID(false);

  beacon.setBeaconID(beaconID);

  Serial.begin(9600);

  analogReference(VDD);
  pinMode(LED, OUTPUT);
  pinMode(PA4, INPUT_PULLUP);
  pinMode(PA7, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(BTN1), handleButtonInterrupt, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PA4), handleExternalTrigger, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PA7), handleExternalTrigger, CHANGE);

  button.attachClick(onClick);
  button.attachLongPressStart(onLongPress);
  button.attachDoubleClick(onLongPress);

  blink(LED);
  delay(500);

  LoRa.setPins(NSS, NRESET, DI01);
  LoRa.setSpreadingFactor(SpreadingFactor);
  LoRa.setSignalBandwidth(SignalBandwidth);
  LoRa.setSyncWord(SyncWord);
  LoRa.setTxPower(TxPower);
  
  delay(2000);

  if (!LoRa.begin(BAND)) {
    while (1) {
      blinkSlow(LED);
    }
  }

  Serial.println("Ready");

  blink(LED, 3);
}



void loop() {
  button.tick();

  switch (beaconMode)  {
  case TRANSMIT:
    if (shouldTransmit) {
      float val = float(analogReadEnh(VSENS, 12)) / float(4096);
      float miliVolts = val * voltageMultiplier + voltageIntercept;

      // When the device is connected via the programming header,
      // the voltage reading will be off
      if (miliVolts < 0) {
        miliVolts = 0;
      }

      if (beacon.transmit(digitalRead(PA4), digitalRead(PA7), digitalRead(BTN1), (uint16_t)miliVolts)) {
        blink(LED, 2);
      }

      shouldTransmit = false;
    }

    // Stay awake to correctly handle long button press
    if (millis() - wokeUpAt > 2000) {
      sleep();
    }

    break;

  case RESET:
    initBeaconID(true);
    beaconMode = TRANSMIT;
    blinkSlow(5);

  default:
    // This should never happen
    break;
  }
}
