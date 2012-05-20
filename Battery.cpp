#include "Battery.h"

Battery::Battery() {
  batteryVoltagePin = A7;
  chargedPin = 7;
  chargingPin = 6;
  pinMode(batteryVoltagePin, INPUT);
  pinMode(chargedPin, INPUT);
  pinMode(chargingPin, INPUT);
}

float Battery::getVoltage() {
  batteryVoltage = analogRead(batteryVoltagePin) * 0.00322265625 * 6;
  return batteryVoltage;
}

boolean Battery::isCharging() {
  return digitalRead(chargingPin) == HIGH;
}

boolean Battery::isCharged() {
  return digitalRead(chargedPin) == LOW;
}
