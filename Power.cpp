#include "Power.h" 

Power::Power()
{
}

// Interrupt service routine for external interrupt on INT0 pin conntected to DS3231 /INT
void INT0_ISR()
{
  //Keep this as short as possible. Possibly avoid using function calls
  detachInterrupt(0);
}

void Power::initSleepMode()
{
  // Initialize INT0 pin for accepting interrupts
  PORTD |= 0x04; 
  DDRD &=~ 0x04;
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  
  RTC.begin();
  lastWakeup = RTC.now();
}

void Power::sleepFor(int seconds)
{
  sleepUntil(DateTime(RTC.now().get() + seconds));
}

void Power::sleepEvery(int seconds)
{
  sleepUntil(DateTime(lastWakeup.get() + seconds));
}

void Power::sleepUntil(DateTime wakeupTime)
{
  RTC.clearINTStatus(); //This function call is  a must to bring /INT pin HIGH after an interrupt.
  RTC.enableInterrupts(wakeupTime.hour(), wakeupTime.minute(), wakeupTime.second());    // set the interrupt at (h,m,s)
  attachInterrupt(0, INT0_ISR, LOW);  //Enable INT0 interrupt (as ISR disables interrupt). This strategy is required to handle LEVEL triggered interrupt
  
  // Power Down routines
  cli(); 
  sleep_enable();      // Set sleep enable bit
  sleep_bod_disable(); // Disable brown out detection during sleep. Saves more power
  sei();
        
  //Serial.println("\nSleeping");
  delay(10);           // This delay is required to allow print to complete
  // Shut down all peripherals like ADC before sleep. Refer Atmega328 manual
  power_all_disable(); // This shuts down ADC, TWI, SPI, Timers and USART
  sleep_cpu();         // Sleep the CPU as per the mode set earlier(power down)  
  sleep_disable();     // Wakes up sleep and clears enable bit. Before this ISR would have executed
  power_all_enable();  // This shuts enables ADC, TWI, SPI, Timers and USART
  
  lastWakeup = RTC.now();
  
  delay(10);           // This delay is required to allow CPU to stabilize
}
