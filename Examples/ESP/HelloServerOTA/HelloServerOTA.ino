#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
ADC_MODE(ADC_VCC); // to use getVcc
#include <ArduinoOTA.h>
#include "FS.h" // SPIFFS File System
#include "otahelp.h"  // OTA code loading setup from BasicOTA
void setupOTA( void );

#include "userssid.h"   // Keep SSID and PWD out of github code base
//const char* ssid = "SSID";
//const char* password = "pwd";

ESP8266WebServer server(80);

char  szBuild[] = "OTA_FS_1.0.6"; // simple debug version string to verify new uploads

void SysInfo( void );   // update sysinfo String
void ledTog( uint8_t ledCMD  );  // toggle RGB
void handleNotFound( void ); // Server page 404 > 200
void SeeSPIFFS(void);

const int ESTOP_PIN = 4;  // If this pin HIGH in setup() then only OTA code runs
const int Bled = 12;
const int Rled = 15;  // 14 on ESP and 15 on Serial3
const int Gled = 13;
#define RGB_LEDS 3
uint8_t ledsRGB[ RGB_LEDS ] { Gled, Bled, Rled  };

uint32_t G40cnt = 0;
uint32_t ts0 = 0;
uint32_t ts1 = 0;
String webString = "";   // String to display
String sysString = "";   // String to display
String FirstHeap = "";
void handleRoot() {
  digitalWrite(Rled, 1);
  //  server.send(200, "text/plain", "hello from esp8266!");
  webString = "Hello World! Reset setup() time:" + String(ts0) + "  Time to WiFi Start Millis:" + String(ts1 - ts0)   + "\nCurrent millis=" + String(millis()) + sysString;
  server.send(200, "text/plain", webString);            // send to someones browser when asked
  digitalWrite(Rled, 0);
}

void handleBig() {
  digitalWrite(Rled, 1);
  //  server.send(200, "text/plain", "hello from esp8266!");
  String bigString = "Hello World! Reset setup() time:" + String(ts0) + "  Time to WiFi Start Millis:" + String(ts1 - ts0)   + "\nCurrent millis=" + String(millis()) + sysString;

  bigString += "\n";
  for ( int ii = 0; ii < 26; ii++ ) {
    for ( int jj = 0; jj < 100; jj++ ) {
      bigString += (char)( ii + 65);
    }
    bigString += "\n";
  }
  server.send(200, "text/plain", bigString);            // send to someones browser when asked
  bigString = "";
  digitalWrite(Rled, 0);
}

int ttog = 0;
uint16_t waitVal = 500;
uint32_t lm = 0;
uint32_t lm2 = 0;
uint32_t mmiss = 0;
uint32_t amiss = 0; // amplitude of missed millis

void ledTog( uint8_t ledCMD ) {
  int8_t ii;

  if ( 255 == ledCMD ) {
    for ( ii = 0; ii < RGB_LEDS; ii++ ) {
      digitalWrite(ledsRGB[ii], LOW);
      pinMode(ledsRGB[ii], OUTPUT);
    }
    digitalWrite(ledsRGB[RGB_LEDS - 1], HIGH);
    return;
  }

  if ( millis() > (lm2 + 1) ) {
    amiss += (millis() - (lm2 + 1));
    mmiss += 1;
  }
  lm2 = millis();
  if ( !(millis() % waitVal) && millis() != lm ) {
    lm = millis();  // On Teensy it would hit multiple times per ms
    waitVal = 1 + (100 * ((millis() / 10000) % 10));
    if (ttog % 2) {
      digitalWrite(ledsRGB[0], LOW);
      digitalWrite(ledsRGB[1], HIGH);
      Serial.print("B.");
    }
    else {
      digitalWrite(ledsRGB[0], HIGH);
      digitalWrite(ledsRGB[1], LOW);
      digitalWrite(ledsRGB[2], LOW);
      Serial.print("G.");
    }
    ttog += 1;
    if ( ttog > 39 ) {
      digitalWrite(ledsRGB[0], LOW);
      digitalWrite(ledsRGB[1], LOW);
      digitalWrite(ledsRGB[2], HIGH);
      Serial.print("\nGreen ::Missed ms=");
      Serial.print( mmiss );
      Serial.print(" ::ms missed=");
      Serial.print( amiss );
      Serial.print(" ::Wait Val=");
      Serial.print( waitVal);
      G40cnt++;
      Serial.print(" :: Green NL CNT=");
      Serial.print( G40cnt);
      Serial.print("   Millis:");
      Serial.println(millis());
      if ( amiss - mmiss > 1000 ) {
        Serial.print("\n -----  ::LONG ms missed=");
        Serial.println( amiss - mmiss );
      }
      mmiss = 0;
      amiss = 0;
      ttog = 0;
      if ( !(millis() % 4)) {
        SysInfo();
      }
    }
  }
}

void handleNotFound() {
  digitalWrite(Rled, 1);
  webString = "File Not Found\n\n";
  webString += "URI: ";
  webString += server.uri();
  webString += "\nMethod: ";
  webString += (server.method() == HTTP_GET) ? "GET" : "POST";
  webString += "\nArguments: ";
  webString += server.args();
  webString += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    webString += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  //  server.send(404, "text/plain", message);
  server.send(200, "text/plain", webString);
  digitalWrite(Rled, 0);
}

int16_t EStop = 0;
// Put any risky Startup code - or one time run code here to prevent loss of OTA
void setup2(void) {
  SeeSPIFFS();
}


void setup(void) {
  ts0 = millis();
  if ( digitalRead( ESTOP_PIN ) ) {
    EStop = 1;
    Serial.setDebugOutput(true);  // OPTIONAL
  }
  ledTog( 255 );

  // Serial.begin(74880);
  Serial.begin(115200);
  webString.reserve(360);
  sysString.reserve(260);
  FirstHeap.reserve(40);
  while (!Serial && (millis() <= 3000));
  if ( EStop  ) {
    Serial.println();
    Serial.print( ESTOP_PIN );
    Serial.print("::ESTOP PIN HIGH :: OTA ONLY !!!!");
  }

  WiFi.begin(ssid, password);
  Serial.println("\nWiFi begin:");
  Serial.println(szBuild);
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    ledTog( 0 );
    Serial.print(".");
  }
  ts1 = millis();
  setupOTA(); // Call BasicOTA setup
  Serial.print("\nConnected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  SysInfo();
  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }
  waitVal = 1 + (100 * ((millis() / 10000) % 10));

  server.on("/", handleRoot);
  server.on("/t", []() { // if you add this subdirectory to your webserver call, you get text below :)
    webString = "Millis: " + String((int)millis()) + "\nToggle: " + String((int)ttog);
    server.send(200, "text/plain", webString);            // send to someones browser when asked
  });
  server.on("/big", handleBig);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
  Serial.print("Time since reset:" ); Serial.print( ts0); Serial.print( "  Time to Start Millis: "); Serial.println( ts1 - ts0);
  lm2 = millis();
  mmiss = 0;
  Serial.print("Sketch size: ");
  Serial.println(ESP.getSketchSize());
  Serial.print("Free size: ");
  Serial.println(ESP.getFreeSketchSpace());
  if ( !EStop ) {
    // Put any risky Startup code - or one time run code here to prevent loss of OTA
    setup2();
  }
  else Serial.print("___ESTOP PIN HIGH :: OTA ONLY !!!!");
}


void loop(void) {
  if ( !EStop ) {
    ledTog( 0 );
    server.handleClient();
  }
  ArduinoOTA.handle();
}

void SysInfo() {
  // Get some information abut the ESP8266
  uint32_t freeheap = ESP.getFreeHeap();
  if ( 0 == FirstHeap.length() ) FirstHeap = "[ 1st FHSz= " + String(freeheap) + " @" + String(millis()) + "]";
  sysString = "\nFree Heap Size= " + String(freeheap) + FirstHeap;
  uint32_t chipID = ESP.getChipId();
  sysString +=   "\nESP8266 chip ID= " + String(chipID);
  uint32_t flashChipID = ESP.getFlashChipId();
  sysString +=   "\n ID = " + String(flashChipID);
  uint32_t flashChipSize = ESP.getFlashChipSize();
  sysString +=   "\n sz= " + String(flashChipSize);
  sysString +=   " bytes";
  uint32_t flashChipSpeed = ESP.getFlashChipSpeed();
  sysString +=   "\n speed = " + String(flashChipSpeed);
  sysString +=   " Hz";
  uint32_t getVcc = ESP.getVcc();
  sysString +=   "\n supply voltage = " + String(getVcc);
  sysString +=   "V";
  sysString +=   "\nSketch sz:" + String(ESP.getSketchSize());
  sysString +=   "\nFree sz:" + String(ESP.getFreeSketchSpace());
  sysString +=   "\nIP#: " + String(WiFi.localIP());
  sysString +=   "\nLast SysInfo Update @" + String(millis()) + "\n";
  sysString +=   "\n BldID:" + String(szBuild);

  Serial.print(FirstHeap.length());
  Serial.println( "str len First");
  Serial.print(sysString.length());
  Serial.println( "str len sys");
  Serial.print(webString.length());
  Serial.println( "str len web");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print(" BldID:");
  Serial.println(szBuild);
}



void SeeSPIFFS() {
  if ( SPIFFS.begin() ) {
    Serial.println("SPIFFS TRUE");
    File file = SPIFFS.open("/here.txt", "r");
    int ii = 0;
    char cbuffer[100] = "";
    int jj;
    int kk;
    if (file) {
      Serial.println("file here.txt found!");
      jj = file.size();
      while ( ii < jj ) {
        if ( (jj - ii) > 99 ) kk = 99; else kk = jj - ii;
        file.readBytes(cbuffer, kk);
        cbuffer[kk] = 0;
        Serial.print(cbuffer);
        ii += kk;
      }
      file.close();
      Serial.println("\n-------");
    }

    file = SPIFFS.open( "/hex/BaseBlinkT31.hex", "r");
    if (file) {
      ii = 0;
      Serial.println("file /hex/BaseBlinkT31.hex found!");
      jj = 400; // file.size();
      while ( ii < jj ) {
        if ( (jj - ii) > 99 ) kk = 99; else kk = jj - ii;
        file.readBytes(cbuffer, kk);
        cbuffer[kk] = 0;
        Serial.print(cbuffer);
        ii += kk;
      }
      file.close();
      Serial.println("\n-------");
    }

    Serial.println("DIR /");
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {
      Serial.print(dir.fileName());
      file = dir.openFile("r");
      Serial.print(" Len=");
      Serial.println(file.size());
      file.close();  // this not shown in sample ???
    }
    Serial.println("DIR /hex");
    dir = SPIFFS.openDir("/hex");
    while (dir.next()) {
      Serial.print(dir.fileName());
      file = dir.openFile("r");
      Serial.print(" Len=");
      Serial.println(file.size());
      file.close();  // this not shown in sample ???
    }
  }
  else if ( SPIFFS.format() ) {
    Serial.print("SPIFFS FORMATTED");
  }
}

