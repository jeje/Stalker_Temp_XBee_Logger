//Data logger Demonstration using Stalker V2.1 Logs temperature periodically to a file Datalog.csv

//1.Use a FAT16 formatted SD card
//2.Compile and upload the sketch
//3.See if everything works fine using Serial Monitor.
//4.Remove all Serial port code, recompile the sketch and upload.
// This reduces power consumption during battery mode.

#include <Wire.h>
#include <DS3231.h>
#include <Fat16.h>
#include <Fat16util.h>
#include <XBee.h>
#include "Power.h"
#include "Battery.h"

#define DEBUG

int statusLed = 8;
int errorLed = 8;

DS3231 RTC;
Power power;
Battery battery;
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
     pinMode(4, INPUT);    //extern power
     
     pinMode(statusLed, OUTPUT);
     //pinMode(errorLed, OUTPUT);
   
     Wire.begin();
     Serial.begin(57600);
     
     power.initSleepMode();
     
     xbee.begin(9600);
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

#ifdef DEBUG
void flashLed(int pin, int times, int wait) {
    
    for (int i = 0; i < times; i++) {
      digitalWrite(pin, HIGH);
      delay(wait);
      digitalWrite(pin, LOW);
      
      if (i + 1 < times) {
        delay(wait);
      }
    }
}
#endif

void loop () 
{
    ////////////////////// START : Application or data logging code//////////////////////////////////
    RTC.convertTemperature();          //convert current temperature into registers
    float temp = RTC.getTemperature(); //Read temperature sensor value
    
    float voltage = battery.getVoltage();
    
    DateTime now = RTC.now(); //get the current date-time
    
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
    file.print(temp);
    file.print(',');
    file.println(voltage);

    if (!file.close()) 
        error("error closing file");
    //|||||||||||||||||||Write to Disk||||||||||||||||||||||||||||||||||
    
    // Send data to XBee Gateway
    char tempChar[100];
    byte tempBytes[101];
    
    String tempStr = String("At ");
    tempStr += now.year();
    tempStr += "/";
    tempStr += now.month();
    tempStr += "/";
    tempStr += now.date();
    tempStr += ", ";
    tempStr += now.hour();
    tempStr += ":";
    tempStr += now.minute();
    tempStr += ":";
    tempStr += now.second();
    tempStr += ' ';
    tempStr += ftoa(tempChar, temp, 1);
    tempStr += " C";
    tempStr += "\n";
    tempStr += "Battery is ";
    tempStr += ftoa(tempChar, voltage, 2);
    tempStr += "V, charging: ";
    tempStr += battery.isCharging() ? "TRUE" : "FALSE";
    tempStr += ", charged: ";
    tempStr += battery.isCharged() ? "TRUE" : "FALSE";
    tempStr += "\n";
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
                #ifdef DEBUG
             	flashLed(statusLed, 5, 100);
                #endif
           } else {
                // the remote XBee did not receive our packet. is it powered on?
                #ifdef DEBUG
             	flashLed(errorLed, 2, 500);
                #endif
           }
        }      
    } else if (xbee.getResponse().isError()) {
      //nss.print("Error reading packet.  Error code: ");  
      //nss.println(xbee.getResponse().getErrorCode());
      // or flash error led
    } else {
      // local XBee did not provide a timely TX Status Response.  Radio is not configured properly or connected
      #ifdef DEBUG
      flashLed(errorLed, 2, 50);
      #endif
    }
    
    power.sleepEvery(10);      // sleep for 10 seconds since last wakeup
}

