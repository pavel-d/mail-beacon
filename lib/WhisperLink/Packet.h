#ifndef PACKET_H
#define PACKET_H

#include <Arduino.h>

#define PACKET_TYPE_TRIGGER 0x01
#define PACKET_TYPE_TEMPERATURE 0x02
#define PACKET_TYPE_HUMIDITY 0x03

#define PACKET_BEACON_ID_SIZE 4
#define PACKET_HEADER_SIZE 6 // beacon ID (4), beaconType (1) and dataLength (1)

#define DATA_SIZE_TRIGGER 7

#define PACKET_SIZE_TRIGGER PACKET_HEADER_SIZE + DATA_SIZE_TRIGGER

struct BeaconPacket {
  uint8_t beaconID[4]; // Unique beacon ID
  uint8_t beaconType;  // Identifier for the type of sensor
  uint8_t dataLength;  // Length of the data. LoRa supports 256 bytes max, uint8_t should be enough
  uint8_t *data;

  int16_t rssi; // Only popupated by the gateway
  float snr;
};

bool parsePacket(const byte* buffer, size_t packetSize, BeaconPacket &packet);
void encodeHeader(byte* packetBuffer, byte* beaconID, uint8_t packetType, uint8_t dataLength);

void encodeTriggerPacket(byte* buffer, byte* beaconID, bool triggerA, bool triggerB, bool triggerC, uint16_t voltage, uint16_t counter);

void deallocateBeaconData(BeaconPacket &packet);

#endif