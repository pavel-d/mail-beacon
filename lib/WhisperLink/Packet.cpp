#include "Packet.h"
#include <Arduino.h>


bool parsePacket(const byte* buffer, size_t packetSize, BeaconPacket &packet) {
  memcpy(&packet.beaconID, buffer, sizeof(uint16_t));

  packet.beaconType = buffer[1];
  packet.dataLength = buffer[2];

  if (packet.dataLength + PACKET_HEADER_SIZE > packetSize) {
    Serial.printf("Unexpected data length: %d\n", packet.dataLength);
    return 0;
  }

  packet.data = new uint8_t[packet.dataLength];

  memcpy(packet.data, buffer + PACKET_HEADER_SIZE, packet.dataLength);

  return 1;
}

void encodeTriggerPacket(byte* packetBuffer, byte* beaconID, bool triggerA, bool triggerB, bool triggerC, uint16_t voltage, uint16_t counter) {
  encodeHeader(packetBuffer, beaconID, PACKET_TYPE_TRIGGER, DATA_SIZE_TRIGGER);

  memcpy(packetBuffer + PACKET_HEADER_SIZE, &triggerA, sizeof(bool));
  memcpy(packetBuffer + PACKET_HEADER_SIZE + sizeof(bool), &triggerB, sizeof(bool));
  memcpy(packetBuffer + PACKET_HEADER_SIZE + sizeof(bool) + sizeof(bool), &triggerC, sizeof(bool));
  memcpy(packetBuffer + PACKET_HEADER_SIZE + sizeof(bool) + sizeof(bool) + sizeof(bool), &voltage, sizeof(uint16_t));
  memcpy(packetBuffer + PACKET_HEADER_SIZE + sizeof(bool) + sizeof(bool) + sizeof(bool) + sizeof(uint16_t), &counter, sizeof(uint16_t));
}

void encodeHeader(byte* packetBuffer, byte* beaconID, uint8_t packetType, uint8_t dataLength) {
  memcpy(packetBuffer, beaconID, PACKET_BEACON_ID_SIZE);
  memcpy(packetBuffer + PACKET_BEACON_ID_SIZE, &packetType, sizeof(uint8_t));
  memcpy(packetBuffer + PACKET_BEACON_ID_SIZE + sizeof(uint8_t), &dataLength, sizeof(uint8_t));
}

void deallocateBeaconData(BeaconPacket &packet) {
    if (packet.data != nullptr) {
        delete[] packet.data;
        packet.data = nullptr;
    }
}