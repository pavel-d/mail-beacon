#include <Arduino.h>
#include <LoRa.h>
#include "Beacon.h"
#include "Packet.h"

Beacon::Beacon() {
}

bool Beacon::listen() {
  int packetSize = LoRa.parsePacket();

  if (!packetSize) {
    return 0;
  }

  return 1;
}

void Beacon::setBeaconID(unsigned long id) {
  beaconID = id;
}

bool Beacon::transmit(bool triggerA, bool triggerB, bool triggerC, uint16_t voltage) {
  if (beaconID == 0) {
    return 0;
  }

  if (!sendMsg(beaconID, triggerA, triggerB, triggerC, voltage, counter++)) {
    return 0;
  }

  return 1;
}

void printByteArrayAsHex(uint8_t arr[], int length) {
  for (int i = 0; i < length; i++) {
    if (arr[i] < 0x10) {
      Serial.print("0");
    }
    Serial.print(arr[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
}

int Beacon::sendMsg(uint16_t beaconID, bool triggerA, bool triggerB, bool triggerC, uint16_t voltage, uint16_t counter) {
  // Serial.printf("Encoding: %X %u %u\n", beaconID, counter, voltage);

  uint8_t outputBuffer[PACKET_SIZE_TRIGGER];

  encodeTriggerPacket(outputBuffer, beaconID, triggerA, triggerB, triggerC, voltage, counter);

  // Serial.print("Payload: ");
  // printByteArrayAsHex(outputBuffer, sizeof(outputBuffer));

  // send packet
  if (!LoRa.beginPacket()) {
    return 0;
  }

  LoRa.write(outputBuffer, sizeof(outputBuffer));
 
  return LoRa.endPacket();
}