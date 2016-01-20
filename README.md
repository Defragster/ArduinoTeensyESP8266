Using Arduino on Teensy and ESP8266

Work in Progress

https://github.com/esp8266/arduino

https://github.com/esp8266/Arduino/blob/master/doc/ota_updates/ota_updates.md#arduinoota

Connected and powered by a PJRC Teensy 3.2.

ESP units are a 12E and a onehorse unit from Tindie : https://www.tindie.com/products/onehorse/esp8266-add-on-for-teensy-32/


First "HelloServerOTA" example for ESP8266
--- Started with HelloServer ESP example then merged added OTA code from BasicOTA example
> Edit the userssid.h file in the sketch folder to supply your network SSID and password 
> The sketch cycles power to GPIO 12,13,15  where I stuck an RGB LED, it puts out serial data, presents web text.
> Incorporates Arduino OTA Port based uploading
> Use build for your ESP unit, I set mine to 1M ( 256K SPIFFS ) to allow OTA code to fit at 259K it is 33% code space

First AutoRefresh "FirstPage" Web Page set: start index.htm
---Sample auto update browser script to monitor ESP8266
> Edit menu.htm for starting IP if you have a fixed one to save editing it (include subpage if you make one like /t )
  >> edit :: ESP IP: <input type="text" name="espip" value="192.168.1.100">
> Press LOAD to show the web page in lower frame
> Press AUTO button for 2500ms auto refresh, edit menu.htm to adjust timing 500ms works, when no multi unit collisions
  >> setInterval(loadeip, 2500) 

  
  Sample web output:
  Hello World! Reset setup() time:3497  Time to WiFi Start Millis:4514
Current millis=1601225
Free Heap Size= 37824[ 1st FHSz= 38144 @8013]
ESP8266 chip ID= 850894
 ID = 1327343
 sz= 1048576 bytes
 speed = 40000000 Hz
 supply voltage = 3410V
Sketch sz:262912
Free sz:499712
IP#: 1677830336
Last SysInfo Update @1533898

 BldID:OTA_1.0.1
