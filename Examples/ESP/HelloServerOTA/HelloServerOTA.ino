#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "userssid.h"
ADC_MODE(ADC_VCC); // to use getVcc
#include <ArduinoOTA.h>

//const char* ssid = "SSID";
//const char* password = "pwd";

ESP8266WebServer server(80);

char  szBuild[] = "OTA_1.0.1";

void SysInfo( void );   // update sysinfo Strinh
void do_ttog ( void );  // toggle RGB
void handleNotFound( void ); // Server page 404 > 200
void setupOTA( void );


const int Gled = 12;
const int Bled = 15;
const int Rled = 13;

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

int ttog = 0;
uint16_t waitVal = 500;
uint32_t lm = 0;
uint32_t lm2 = 0;
uint32_t mmiss = 0;
void do_ttog () {
  if ( millis() > (lm2 + 1) ) mmiss += 1;
  lm2 = millis();
  if ( !(millis() % waitVal) && millis() != lm ) {
    lm = millis();  // On Teensy it would hit multiple times per ms
    waitVal = 1 + (100 * ((millis() / 10000) % 10));
    if (ttog % 2) {
      digitalWrite(Rled, LOW);
      digitalWrite(Bled, HIGH);
      Serial.print("B.");
    }
    else {
      digitalWrite(Gled, LOW);
      digitalWrite(Bled, LOW);
      digitalWrite(Rled, HIGH);
      Serial.print("R.");
    }
    ttog += 1;
    if ( ttog > 39 ) {
      digitalWrite(Rled, LOW);
      digitalWrite(Bled, LOW);
      digitalWrite(Gled, HIGH);
      Serial.print("\nGreen :: Missed millis=");
      Serial.print( mmiss );
      Serial.print(" :: Wait Value=");
      Serial.print( waitVal);
      G40cnt++;
      Serial.print(" :: Green NL CNT=");
      Serial.print( G40cnt);
      Serial.print("   Millis:");
      Serial.println(millis());
      mmiss = 0;
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

void setup(void) {
  ts0 = millis();
  digitalWrite(Rled, LOW);
  pinMode(Rled, OUTPUT);
  digitalWrite(Bled, LOW);
  pinMode(Bled, OUTPUT);
  digitalWrite(Gled, HIGH);
  pinMode(Gled, OUTPUT);
  // Serial.begin(74880);
  Serial.begin(115200);
  webString.reserve(360);
  sysString.reserve(260);
  FirstHeap.reserve(40);
  while (!Serial && (millis() <= 3000));

  WiFi.begin(ssid, password);
  Serial.println("\nWiFi begin:");
  Serial.println(szBuild);
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    do_ttog();
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
}

void loop(void) {
  do_ttog();
  server.handleClient();
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
  Serial.print("\n BldID:");
  Serial.print(szBuild);
}


// Serial & WiFi online
void setupOTA() {
  Serial.println("Booting (OTA)");
  /*

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.waitForConnectResult() != WL_CONNECTED) {
      Serial.println("Connection Failed! Rebooting...");
      delay(5000);
      ESP.restart();
    }
  */

  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("myesp8266");

  // No authentication by default
  // ArduinoOTA.setPassword((const char *)"123");

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("End");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\n\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

