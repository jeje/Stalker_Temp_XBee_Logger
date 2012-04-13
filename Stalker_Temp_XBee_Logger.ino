//Data logger Demonstration using Stalker V2.1 Logs temperature periodically to a file Datalog.csv

//1.Use a FAT16 formatted SD card
//2.Compile and upload the sketch
//3.See if everything works fine using Serial Monitor.
//4.Remove all Serial port code, recompile the sketch and upload.
// This reduces power consumption during battery mode.

#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/power.h>
#include <Wire.h>
#include <DS3231.h>
#include <Fat16.h>
#include <Fat16util.h>
#include <XBee.h>

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


DS3231 RTC; //Create RTC object for DS3231 RTC come Temp Sensor 
static uint8_t prevSecond=0; 
static DateTime interruptTime;
SdCard card;
Fat16 file;

// XBee Configuration -- send data to Xbee router
XBee xbee;
XBeeAddress64 gateway = XBeeAddress64(0x00000000, 0x0000FFFF);
TxStatusResponse txStatus = TxStatusResponse();

// store error strings in flash to save RAM
#define error(s) error_P(PSTR(s))

void error_P(const char* str) {
  PgmPrint("error: ");
  SerialPrintln_P(str);
  if (card.errorCode) {
    PgmPrint("SD error: ");
    Serial.println(card.errorCode, HEX);
  }
  while(1);
}


void setup () 
{
     /*Initialize INT0 pin for accepting interrupts */
     PORTD |= 0x04; 
     DDRD &=~ 0x04;
     pinMode(4,INPUT);//extern power
   
     Wire.begin();
     Serial.begin(57600);
     RTC.begin();
     xbee.begin(9600);
     
     attachInterrupt(0, INT0_ISR, LOW); //Only LOW level interrupt can wake up from PWR_DOWN
     set_sleep_mode(SLEEP_MODE_PWR_DOWN);
 
     //Enable Interrupt 
     RTC.enableInterrupts(EveryMinute); //interrupt at  EverySecond, EveryMinute, EveryHour
}

char *ftoa(char *a, double f, int precision)
{
  long p[] = {0,10,100,1000,10000,100000,1000000,10000000,100000000};
  
  char *ret = a;
  long heiltal = (long)f;
  itoa(heiltal, a, 10);
  while (*a != '\0') a++;
  *a++ = '.';
  long desimal = abs((long)((f - heiltal) * p[precision]));
  itoa(desimal, a, 10);
  return ret;
}

void loop () 
{
    ////////////////////// START : Application or data logging code//////////////////////////////////
    RTC.convertTemperature();          //convert current temperature into registers
    float temp = RTC.getTemperature(); //Read temperature sensor value
    
    DateTime now = RTC.now(); //get the current date-time    
    if((now.second()) !=  prevSecond )
    {
    //print only when there is a change
    //Serial.print(now.year(), DEC);
    //Serial.print('/');
    //Serial.print(now.month(), DEC);
    //Serial.print('/');
    //Serial.print(now.date(), DEC);
    //Serial.print(' ');
    //Serial.print(now.hour(), DEC);
    //Serial.print(':');
    //Serial.print(now.minute(), DEC);
    //Serial.print(':');
    //Serial.print(now.second(), DEC);
    //Serial.print("   ");
    //Serial.print(temp);
    //Serial.print(" deg C");
    //Serial.println();
    }
     prevSecond = now.second();
    
    //|||||||||||||||||||Write to Disk||||||||||||||||||||||||||||||||||
    // initialize the SD card
    if (!card.init()) error("card.init");
  
    // initialize a FAT16 volume
    if (!Fat16::init(&card)) error("Fat16::init");
  
    char name[] = "DATALOG.CSV";
    // clear write error
    file.writeError = false;
  
    // O_CREAT - create the file if it does not exist
    // O_APPEND - seek to the end of the file prior to each write
    // O_WRITE - open for write
    if (!file.open(name, O_CREAT | O_APPEND | O_WRITE))
        error("error opening file");

    file.print(now.year(), DEC);
    file.print('/');
    file.print(now.month(), DEC);
    file.print('/');
    file.print(now.date(), DEC);
    file.print(',');
    file.print(now.hour(), DEC);
    file.print(':');
    file.print(now.minute(), DEC);
    file.print(':');
    file.print(now.second(), DEC);
    file.print(',');
    file.println(temp);

    if (!file.close()) 
        error("error closing file");
    //|||||||||||||||||||Write to Disk||||||||||||||||||||||||||||||||||
    
    // Send data to XBee Gateway
    char tempChar[40];
    byte tempBytes[41];
    
    String tempStr = String("Temperature is: ");
    tempStr += ftoa(tempChar, temp, 1);
    tempStr += " C\n";
    tempStr.getBytes(tempBytes, sizeof(tempBytes));
    
    Tx64Request tx = Tx64Request(gateway, tempBytes, tempStr.length());
    xbee.send(tx);
    
    // after sending a tx request, we expect a status response
    // wait up to 5 seconds for the status response
    if (xbee.readPacket(5000)) {
        // got a response!           	
    	if (xbee.getResponse().getApiId() == TX_STATUS_RESPONSE) {
    	   xbee.getResponse().getZBTxStatusResponse(txStatus);	
    	   // get the delivery status, the fifth byte
           if (txStatus.getStatus() == SUCCESS) {
            	// success.  time to celebrate
           } else {
            	// the remote XBee did not receive our packet. is it powered on?
           }
        }      
    } else if (xbee.getResponse().isError()) {
      //nss.print("Error reading packet.  Error code: ");  
      //nss.println(xbee.getResponse().getErrorCode());
      // or flash error led
    } else {
      // local XBee did not provide a timely TX Status Response.  Radio is not configured properly or connected
    }
    
    
    RTC.clearINTStatus(); //This function call is  a must to bring /INT pin HIGH after an interrupt.
    attachInterrupt(0, INT0_ISR, LOW);  //Enable INT0 interrupt (as ISR disables interrupt). This strategy is required to handle LEVEL triggered interrupt
    
    
    ////////////////////////END : Application code //////////////////////////////// 
   
    
    //\/\/\/\/\/\/\/\/\/\/\/\/Sleep Mode and Power Down routines\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\
            
    //Power Down routines
    cli(); 
    sleep_enable();      // Set sleep enable bit
    sleep_bod_disable(); // Disable brown out detection during sleep. Saves more power
    sei();
        
    //Serial.println("\nSleeping");
    delay(10); //This delay is required to allow print to complete
    //Shut down all peripherals like ADC before sleep. Refer Atmega328 manual
    power_all_disable(); //This shuts down ADC, TWI, SPI, Timers and USART
    sleep_cpu();         // Sleep the CPU as per the mode set earlier(power down)  
    sleep_disable();     // Wakes up sleep and clears enable bit. Before this ISR would have executed
    power_all_enable();  //This shuts enables ADC, TWI, SPI, Timers and USART
    delay(10); //This delay is required to allow CPU to stabilize
    //Serial.println("Awake from sleep");    
    
    //\/\/\/\/\/\/\/\/\/\/\/\/Sleep Mode and Power Saver routines\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\

} 

  
//Interrupt service routine for external interrupt on INT0 pin conntected to DS3231 /INT
void INT0_ISR()
{
  //Keep this as short as possible. Possibly avoid using function calls
    detachInterrupt(0); 
}

