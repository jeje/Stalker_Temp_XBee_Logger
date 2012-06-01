# XBee temperature and battery usage logger

Arduino Sketch for [Stalker v2.1](http://www.seeedstudio.com/depot/seeeduino-stalker-v2-p-727.html?cPath=80) reporting temperature and battery usage over XBee.
Also logs data in CSV format on the SD-card.


## Requirements

* DS3231 library modified in order to run with Arduino 1.0 -- [see my work at the end of the page](http://localhost:4000/blog/2012/03/12/a-few-stalker-v2-dot-1-glitches-that-can-easily-be-solved/)
* [XBee Library for Arduino](http://code.google.com/p/xbee-arduino/)
* an XBee (Series 1) connected to the computer
* an XBee (Series 1) connected to the Stalker


## FAQ

* How to know if the sketch works fine on my Stalker?

If the XBee Packets are sent properly you should see the user LED to blink 5 times very fast. If it does not work the user LED will only blink fast two times.