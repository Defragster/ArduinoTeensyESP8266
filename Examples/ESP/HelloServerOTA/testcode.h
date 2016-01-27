


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


int ttog = 0;
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


