FOUND THIS SERVER: https://github.com/nailbuster/myWebServer that seems to incorporate all I found and more in a clean library.

The above server proving nice and useful - Participating in new work with Teensy here :: https://github.com/FrankBoesing/FlexiBoard
Working with IDE 1.6.12 and ESP_Arduino Rev 2.3.
Current Update here: https://github.com/FrankBoesing/FlexiBoard/blob/master/extras/ESP8266/README2.ArduinoESP.md

Using Arduino on Teensy and ESP8266

Work in Progress: GOING WELL - multiple ESP units tested with OTA and now Serial program over Teensy USB to Serial

https://github.com/esp8266/arduino

https://github.com/esp8266/Arduino/blob/master/doc/ota_updates/ota_updates.md#arduinoota
* NOTE: Arduino IDE 1.6.7 is the first release that provides support for ArduinoOTA library

This link to working with SPIFFS file system on ESP: http://esp8266.github.io/Arduino/versions/2.1.0-rc1/doc/filesystem.html
 [ newer 0.2.0 working for OTA SPIFFS upload: https://github.com/esp8266/arduino-esp8266fs-plugin/releases ]

Connected and powered by a PJRC Teensy 3.2.

ESP units are a 12E and a onehorse unit from Tindie : https://www.tindie.com/products/onehorse/esp8266-add-on-for-teensy-32/
> A second onehorse unit with PCB to Serial2 used in latest update, can wire to ESP_generic the same way. Initial code (commented out) used a Serial3 PCB unit with wires from Rx/Tx to Teensy ALTERNATE Serial2 pins on a FrankB connectorboard.

Added: Example for Teensy 3.2 (will work on 3.1 with external ESP power) Proxy sketch that routes Serila# to Serial/USB for debug or programming.  USB commands can be issues to: PROGRAM [P :: reset to pgm mode serial], RESET[R}, ESTOP [E=ON, e=off] [Command LETTERS preceded by ` (left apostrophe character). All other USB imput echoed to ESP.

Code functions well - is a mess to read - and poorly commented, some notes here on functionality.

First "HelloServerOTA" example for ESP8266
--- Started with HelloServer ESP example then merged added OTA code from BasicOTA example
* Edit the userssid.h file in the sketch folder to supply your network SSID and password 
* The sketch cycles power to GPIO 12,13,15  where I stuck an RGB LED, it puts out serial data, presents web text.
* Incorporates Arduino OTA Port based uploading
* Use build for your ESP unit, I set mine to 1M ( 256K SPIFFS ) to allow OTA code to fit at 259K it is 33% code space
* [pending] Bad code crashing into wdt (watchdog timer)? - added setup() pin read to skip user code except OTA after WiFii
* Added Serial receive echo back our Seial, it seems SerialEvent not supported so added to loop()
* Any NEW code or code that can CRASH needs to be under 'if (!ESTOP)' - you can then set the declared pin high on Teensy (`E) and reset (`R) to preserve the OTA code ability to recover from crashing resets - I did thie ONCE with a SPIFFS edit, and implemented this failsafe.
* OTA uploads are 3-6 times faster than Serial uploads, and actually seem more reliable
* Check your SPIFFS and other settings in tools as the IDE can reset them - in one case that trashed my OTA code as it shrunk the program area.

First AutoRefresh "FirstPage" Client side Web Page set: start index.htm
---Sample auto update browser script to monitor ESP8266
* Edit menu.htm for starting IP if you have a fixed one to save editing it (include subpage if you make one like /t )
  * >> edit :: ESP IP: <input type="text" name="espip" value="192.168.1.100">
* Press LOAD to show the web page in lower frame
* Press AUTO button for 2500ms auto refresh, edit menu.htm to adjust timing 500ms works, when no multi unit collisions
  * >> setInterval(loadeip, 2500) 

  
  Sample web output:
  Hello World! Reset setup() time:3497  Time to WiFi Start Millis:4514
*Current millis=1601225
*Free Heap Size= 37824[ 1st FHSz= 38144 @8013]
*ESP8266 chip ID= 850894
* ID = 1327343
* sz= 1048576 bytes
* speed = 40000000 Hz
* supply voltage = 3410V
*Sketch sz:262912
*Free sz:499712
*IP#: 1677830336
*Last SysInfo Update @1533898
*
* BldID:OTA_1.0.1
