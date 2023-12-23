#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <PubSubClient.h>

#include "Packet.h"

#define SyncWord 0xF3
#define TxPower 20
#define SignalBandwidth 250E3
#define SpreadingFactor 12
#define BAND 433E6

#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 14
#define DIO0 26

//OLED pins
#define OLED_SDA    21
#define OLED_SCL    22
#define OLED_RST    -1
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

const char* ssid = "My Wifi";
const char* password = "12345678";

const char *mqtt_server = "192.168.1.1";
const int mqtt_port = 31883;


Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);
WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

void blink(int c) {
  for (int i = 0; i <= c; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(25);
    digitalWrite(LED_BUILTIN, LOW);
    delay(25);
  }
}

void blink() {
  blink(1);
}


void blinkSlow() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1500);
  digitalWrite(LED_BUILTIN, LOW);
  delay(1500);
}

void reconnect() {
  while (!mqtt.connected()) {
    Serial.println("Re-connecting to MQTT");

    // Create a random client ID
    String clientId = "LoRaGateway-";
    clientId += String(random(0xffff), HEX);

    if (mqtt.connect(clientId.c_str())) {
    } else {
      Serial.print("MQTT failed, rc=");
      Serial.print(mqtt.state());
      Serial.println(" try again in 1 seconds");
      delay(1000);
    }
  }
}

void printReady() {
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("WhisperLink Receiver");
  display.println("Waiting for data");

  display.display();
}

void printBeaconPacket(BeaconPacket packet, size_t packetSize) {
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("Received packet");

  display.setCursor(0,20);
  display.print("Beacon ID: ");
  display.print(packet.beaconID, HEX);

  display.setCursor(0,30);
  display.printf("Size: %d bytes", packetSize);

  display.setCursor(0,50);
  display.printf("RSSI: %d, SNR: ", LoRa.packetRssi());
  display.print(LoRa.packetSnr());

  display.display();
}

void printBeaconParsingError(size_t packetSize) {
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("Failed to parse beacon packet");

  display.setCursor(0,30);
  display.printf("Size: %d bytes", packetSize);

  display.setCursor(0,50);
  display.printf("RSSI: %d, SNR: %d", LoRa.packetRssi(), LoRa.packetSnr());

  display.display();
}


void setup() {
  pinMode(LED_BUILTIN, OUTPUT);

  //reset OLED display via software
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(20);
  digitalWrite(OLED_RST, HIGH);

  Wire.begin(OLED_SDA, OLED_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false)) { // Address 0x3C for 128x32
    for(;;); // Don't proceed, loop forever
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextWrap(1);
  display.setTextColor(WHITE);

  LoRa.setPins(SS, RST, DIO0);
  LoRa.setSyncWord(SyncWord);
  LoRa.setSpreadingFactor(SpreadingFactor);
  LoRa.setSignalBandwidth(SignalBandwidth);
  LoRa.setTxPower(TxPower);

  mqtt.setServer(mqtt_server, mqtt_port);
  // reconnect();
  
  delay(1000);

  if (!LoRa.begin(BAND)) {
    while (1) {
      blinkSlow();
    }
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED){
    display.clearDisplay();
    display.setCursor(0,0);
    display.println("WhisperLink Receiver");
    display.printf("Connecting to Wi-Fi", ssid);
    display.display();

    delay(100);
  }

  Serial.begin(9600);
  Serial.println("Ready");
  printReady();
  blink(2);
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

void loop() {
  int packetSize = LoRa.parsePacket();

  if (!packetSize) {
    return;
  }

  byte buffer[packetSize + sizeof(int16_t) + sizeof(float)]; // + SNR and RSSI metadata

  for (int i = 0; i < packetSize; i++) {
    buffer[i] = LoRa.read();
  }

  Serial.print("Payload: ");
  printByteArrayAsHex(buffer, sizeof(buffer));

  BeaconPacket parsedPacket;
  if (!parsePacket(buffer, packetSize, parsedPacket)) {
    Serial.printf("Failed to parse packet: (%d bytes)\n", packetSize);
    printBeaconParsingError(packetSize);

    return;
  }

  parsedPacket.rssi = LoRa.packetRssi();
  parsedPacket.snr = LoRa.packetSnr();

  memcpy(buffer + packetSize, &parsedPacket.rssi, sizeof(int16_t));
  memcpy(buffer + packetSize + sizeof(int16_t), &parsedPacket.snr, sizeof(float));

  printBeaconPacket(parsedPacket, packetSize);

  Serial.printf("Received packet (%d bytes)\n", packetSize);

  if (!mqtt.connected()) {
    Serial.println("MQTT not connected");
    reconnect();
  }

  if (!mqtt.connected()) {
    Serial.println("Still not connected");
  }

  if (!mqtt.publish("whisper_link/beacons/raw_events", buffer, sizeof(buffer))) {
    Serial.println("publish() returned false");
  } else {
    Serial.printf("Published (%d bytes)\n", sizeof(buffer));
  }

  if (!mqtt.endPublish()) {
    Serial.println("endPublish() returned false");
  }

  if (!mqtt.loop()) {
    Serial.println("loop() returned false");
  }

  deallocateBeaconData(parsedPacket);

  blink();
}
