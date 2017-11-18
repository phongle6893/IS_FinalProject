
 /* 
 * Typical pin layout used:
 * -----------------------------------------------------------------------------------------
 *             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
 *             Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
 * Signal      Pin          Pin           Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
 * SPI SS      SDA(SS)      10            53        D10        10               10
 * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
 */

#include <SPI.h>
#include <MFRC522.h>

constexpr uint8_t RST_PIN = 9;     // Configurable, see typical pin layout above
constexpr uint8_t SS_PIN = 10;     // Configurable, see typical pin layout above
 
MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class

MFRC522::MIFARE_Key key; 

// Init array that will store new NUID 
byte nuidPICC[4];
byte adminKeys[4] = {0x90, 0x8A, 0xA9, 0x56};
boolean isLogin = false;
boolean isAdminLogin = false;
boolean isEditMode = false;
boolean isScaned = false;

void setup() { 
  Serial.begin(9600);
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522 

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  Serial.println(F("This code scan the MIFARE Classsic NUID."));
  Serial.print(F("Using the following key:"));
  printHex(key.keyByte, MFRC522::MF_KEY_SIZE);
  Serial.println();
}
 
void loop() {
  
  // Look for new cards
  
  if (!rfid.PICC_IsNewCardPresent()) {
    Serial.println("not PICC_IsNewCardPresent");
    return;
  }else {
    Serial.println("IsNewCardPresent");
  }
  if (!rfid.PICC_ReadCardSerial()) {
    Serial.println("not PICC_ReadCardSerial");
    return;
  }else{
    Serial.println("PICC_ReadCardSerial");
  }
  isScaned = true;
  Serial.println("set isScaned");
  checkRole(rfid.uid.uidByte, rfid.uid.size);
  
}

void checkScaned(){
  if (isScaned == true){
    
  }
}

void checkRole(byte *buffer, byte bufferSize) {
  boolean result = true;
  for (byte i = 0; i < bufferSize; i++) {
    if ( result == true ){
      if (buffer[i] != adminKeys[i]) {
        result = false;
      }
    } else {
      Serial.print(F("notAdmin"));
      Serial.println();
      return;
    }
    
  }
    if (result == true){
      if (isAdminLogin == true){
        if( isEditMode == false ) {
          isEditMode = true;
          Serial.println("turn on edit mode");
        }else {
          isEditMode = false;
          isAdminLogin = false;
          Serial.println("turn of edit mode, and admin logout");
        }
      }else {
        isAdminLogin = true;
        Serial.print(F("isAdmin"));
        Serial.println();
      }
    }else{
      Serial.print(F("notAdmin"));
      Serial.println();
    }
}
/**
 * Helper routine to dump a byte array as dec values to Serial.
 */
void printDec(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], DEC);
  }
}
/**
 * Helper routine to dump a byte array as hex values to Serial. 
 */
void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}
