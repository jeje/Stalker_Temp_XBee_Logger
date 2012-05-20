#ifndef STALKER_BATTERY
#define STALKER_BATTERY

#include <Arduino.h>

class Battery
{
  public:
    Battery();
    float getVoltage();
    boolean isCharging();
    boolean isCharged();
  private:
    float batteryVoltage;
    int batteryVoltagePin;    // Analog input pin battery voltage is connected to - usually A7
    int chargedPin;           // Digital pin 7 == charged
    int chargingPin;          // Digital pin 6 == charging
};

#endif
