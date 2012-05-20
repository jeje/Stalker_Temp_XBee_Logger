#ifndef STALKER_POWER
#define STALKER_POWER

#include <Arduino.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <DS3231.h>

class Power
{
  public:
    Power();
    void initSleepMode();
    
    /**
     * Sleep for <tt>seconds</tt> seconds for <tt>RTC.now()</tt>.
     */
    void sleepFor(int seconds);
    /**
     * Sleep every <tt>seconds</tt> seconds since last wake up.
     * This is most likely the methods you'll use if you need to wake up precisely every <tt>seconds</tt> seconds.
     */
    void sleepEvery(int seconds);
    /**
     * Sleep until the specified <tt>date</tt> DateTime.
     */
    void sleepUntil(DateTime date);
  private:
    DS3231 RTC;
    DateTime lastWakeup;
};

//The following code is taken from sleep.h as Arduino Software v22 (avrgcc), in w32 does not have the latest sleep.h file
#define sleep_bod_disable() \
{ \
  uint8_t tempreg; \
  __asm__ __volatile__("in %[tempreg], %[mcucr]" "\n\t" \
                       "ori %[tempreg], %[bods_bodse]" "\n\t" \
                       "out %[mcucr], %[tempreg]" "\n\t" \
                       "andi %[tempreg], %[not_bodse]" "\n\t" \
                       "out %[mcucr], %[tempreg]" \
                       : [tempreg] "=&d" (tempreg) \
                       : [mcucr] "I" _SFR_IO_ADDR(MCUCR), \
                         [bods_bodse] "i" (_BV(BODS) | _BV(BODSE)), \
                         [not_bodse] "i" (~_BV(BODSE))); \
}


#endif
