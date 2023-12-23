#include "Blink.h"
#include <Arduino.h>

void blink(uint8_t pin, int c) {
  for (int i = 0; i <= c; i++) {
    digitalWrite(pin, HIGH);
    delay(25);
    digitalWrite(pin, LOW);
    delay(25);
  }
}

void blink(uint8_t pin) {
  blink(pin, 1);
}

void blinkSlow(uint8_t pin) {
  digitalWrite(pin, HIGH);
  delay(1500);
  digitalWrite(pin, LOW);
  delay(1500);
}