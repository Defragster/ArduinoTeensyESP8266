// PESKY NOT PRESENT
//#define ESP8266_CH_PD      16  //EN
//#define ESP8266_GPIO2      2
//#define ESP8266_GPIO        1
//#define ESP8266_RST        16

// END ROW GPIO :: 4, 2, 15, 13, 12,(14)

char  szBuild[] = "T_3.2_1.0.5";

#define ESP8266_EN        16
#define ESP8266_GPIO16    17
#define ESP8266_ESTOP     17
#define ESP8266_GPIO14    18
// TODO : Need a RUNTIME HAL layer to Adjust these at runtime based on T_3.2 UNIT
// Serial 2 :: This is normal config for New Serial2 onehorse or where Generic ESP8266 should connect
#define ESP8266_PGM0      7
#define ESP8266_ALIVE     8
#define ESP8266_TX        9
// Serial 3 SWAP to Alternate Serial2 :: SPECIAL CASE Unit Serial3 Mapped to ALT_Serial2
// #define ESP8266_PGM0      9
// #define ESP8266_ALIVE     10
// #define ESP8266_TX        31
#define ESP8266_GPIO3     10
#define ESP8266_RX        26

//Serial Ports
#define ESP8266_SERIAL Serial2
#define ESP8266_SERIAL_BAUD_INIT 115200

//  https://forum.pjrc.com/threads/32502-Serial2-Alternate-pins-26-and-31
//  do Serial2.begin(9600): then::

#define qBlink() (digitalWriteFast(LED_BUILTIN, !digitalReadFast(LED_BUILTIN) ))
void doProgram ();  // Put ESP into Program Mode
void ShowESTOP(void ) ;

void setup() {
  pinMode(ESP8266_ESTOP, OUTPUT);
  digitalWrite(ESP8266_ESTOP, LOW);
  ShowESTOP();
  pinMode(ESP8266_ALIVE, INPUT);  // TODO - not implemented
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.begin(9600);
  ESP8266_SERIAL.begin(ESP8266_SERIAL_BAUD_INIT);
  while (!Serial && (millis () <= 3000))
    ;
  Serial.println("Hello ESP!");
  Serial.println(szBuild);

  qBlink();

  if ( 9 == ESP8266_PGM0 ) {  // Set Serial 2 Alternate when PGM pin on Serial2::Tx pin 9
    CORE_PIN9_CONFIG = 0;
    CORE_PIN10_CONFIG = 0;
    CORE_PIN26_CONFIG = PORT_PCR_PE | PORT_PCR_PS | PORT_PCR_PFE | PORT_PCR_MUX(3);
    CORE_PIN31_CONFIG = PORT_PCR_DSE | PORT_PCR_SRE | PORT_PCR_MUX(3);
    Serial.println("ALTERNATE Serial2 @ 26/31");
  }
  else
    Serial.println("Serial2 onehorse & ESP-12E");
}

void doProgram ( uint16_t CMD )
{
  if ( ESP8266_PGM0 == CMD ) {
    pinMode(ESP8266_PGM0, OUTPUT);
    digitalWrite(ESP8266_PGM0, HIGH);
  }
  pinMode(ESP8266_EN, OUTPUT);  //   DOING THIS KILLS SERIAL2_Alternate ?????
  digitalWrite(ESP8266_EN, HIGH);
  if ( ESP8266_PGM0 == CMD ) {
    digitalWrite(ESP8266_PGM0, LOW);
  }
  digitalWrite(ESP8266_EN, LOW);    //Reset:
  delay(2);
  pinMode(ESP8266_EN, INPUT);
  if ( ESP8266_PGM0 == CMD ) {
    delay(400);
    digitalWrite(ESP8266_PGM0, HIGH);
  }
}


void loop() {
}

void serialEvent2() {
  qBlink();
  while (ESP8266_SERIAL.available()) {
    Serial.write( ESP8266_SERIAL.read() );
  }
}


byte incomingByte;
bool pgmMode = false;
void serialEvent() {
  byte cmdDone = 0;
  int ii = 0;
  qBlink();
  if ( false == pgmMode ) {  // MUST NOT PARSE INPUT DURING PROGRAM MODE !!!!!!!
    while (Serial.available()) {
      incomingByte = Serial.read();
      if ( !ii )
        Serial.println("<<<");
      Serial.print("   RCV:");
      ii++;
      Serial.print(incomingByte, DEC);
      if ( '`' == incomingByte ) {
        cmdDone = 1;  // PREFIX first CMD with '`'
      }
      else if ( cmdDone ) {
        if ( 'P' == incomingByte ) {
          cmdDone = 'P';
          Serial.println("Going to PGM MODE.");
          pgmMode = true;
          doProgram(ESP8266_PGM0);
          Serial.println("no more commands accepted:: RESTART TEENSY AFTER PROGRAM UPLOADED !!!!!");
          Serial.println("!!!!!    >>>>   DISABLE SERIAL MONITOR   <<<<<<<<<     !!!!!");
        }
        if ( 'R' == incomingByte ) {
          cmdDone = 'R';
          Serial.println("Doing ESP8266 RESET.");
          pgmMode = false;
          doProgram(ESP8266_EN);
        }
        if ( 'E' == incomingByte ) {
          ShowESTOP();
          Serial.println("Going to ESTOP.");
          digitalWrite(ESP8266_ESTOP, HIGH);
          cmdDone = 'E';
        }
        if ( 'e' == incomingByte ) {
          ShowESTOP();
          Serial.println("Leaving ESTOP.");
          digitalWrite(ESP8266_ESTOP, LOW);
          cmdDone = 'e';
        }
        if ( cmdDone ) {
          delay( 100 );
          qBlink();
        }
      }
      ESP8266_SERIAL.write(incomingByte);
    }
    if ( ii ) Serial.println(">>>");
  }
  else {
    while (Serial.available()) {
      ESP8266_SERIAL.write(Serial.read());
      qBlink();
    }
  }
}


void ShowESTOP() {
  if ( digitalRead( ESP8266_ESTOP ) )
    Serial.println("WAS ESTOP.");
  else
    Serial.println("was NOT ESTOP.");
}

