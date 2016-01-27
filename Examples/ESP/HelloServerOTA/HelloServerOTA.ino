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

// GLOBAL ABUSE
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
char  szBuild[] = "OTA_FS_1.0.7"; // simple debug version string to verify new uploads

void SysInfo( void );   // update sysinfo String
void ledTog( uint8_t ledCMD  );  // toggle RGB
void handleNotFound( void ); // Server page 404 > 200
void SeeSPIFFS(void);


// #include "testcode.h"   // code for testing stuff
uint16_t waitVal = 500;
#include "testcode.h"   // code for testing stuff


ESP8266WebServer server(80);



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


