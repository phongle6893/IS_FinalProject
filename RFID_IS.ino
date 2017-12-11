
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
//#include <Helpers.h>

constexpr uint8_t RST_PIN = 9;     // Configurable, see typical pin layout above
constexpr uint8_t SS_PIN = 10;     // Configurable, see typical pin layout above
 
MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class

MFRC522::MIFARE_Key key; 

// Init array that will store new NUID 
byte nuidPICC[4];
byte adminKey[4] = {0x02, 0x0F, 0x0F, 0xA9};
byte userKeys[4][4]={
  {0x02, 0x0F, 0x0F, 0xA9},
  {0x41, 0xDC, 0xB5, 0x79}, //41 DC B5 79
  {0x10, 0x5D, 0x80, 0xA5}, //10 5D 80 A5
  {0x90, 0xA8, 0xA9, 0x56}  //90 8A A9 56
};
int roles[4] = {31,20,0,0};
int option[32][5] = {
  {0,0,0,0,0},
  {0,0,0,0,1},
  {0,0,0,1,0},
  {0,0,0,1,1},
  {0,0,1,0,0},
  {0,0,1,0,1},
  {0,0,1,1,0},
  {0,0,1,1,1},
  {0,1,0,0,0},
  {0,1,0,0,1},
  {0,1,0,1,0},
  {0,1,0,1,1},
  {0,1,1,0,0},
  {0,1,1,0,1},
  {0,1,1,1,0},
  {0,1,1,1,1},
  {1,0,0,0,0},
  {1,0,0,0,1},
  {1,0,0,1,0},
  {1,0,0,1,1},
  {1,0,1,0,0},
  {1,0,1,0,1},
  {1,0,1,1,0},
  {1,0,1,1,1},
  {1,1,0,0,0},
  {1,1,0,0,1},
  {1,1,0,1,0},
  {1,1,0,1,1},
  {1,1,1,0,0},
  {1,1,1,0,1},
  {1,1,1,1,0},
  {1,1,1,1,1},
};
//int option[8][3] = {
//  {1,1,1},
//  {1,1,0},
//  {1,0,1},
//  {1,0,0},
//  {0,1,1},
//  {0,1,0},
//  {0,0,1},
//  {0,0,0}
//};
int select = -1;
boolean isLogin     = false;
boolean isAdminLogin= false;
boolean isEditMode  = false;
boolean isScaned    = false;
boolean isEditing   = false;
int lastRole = 0;
// Led pins
int led[5] = {4,5,6,7,8};

void setup() { 
  Serial.begin(9600);
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522 
  for (int i = 0; i < sizeof(led); i++){
    pinMode(led[i], OUTPUT);
  }
  
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
  if (isEditMode == true && isEditing == false){
    blinks();
  }
  if (!rfid.PICC_IsNewCardPresent()) {
      return; 
  }
  if (!rfid.PICC_ReadCardSerial()) {
    return;
  }
  
  checkRole();
  prln("-------------");
  delay(1000);
}

void checkRole(){
  if (compare(rfid.uid.uidByte, adminKey, rfid.uid.size)){
    // is admin
    Serial.println("is admin");
    showOptionLed(roles[findRole()]);
    
    if (isAdminLogin == true){
      
      if (isEditMode == false){
        // switch to edit mode
        isEditMode = true;
        prln("switch to edit mode");
      }else{
        // End editing
        if(select >= 0 ){
          roles[lastRole] = select;
          Serial.print("select option");
          Serial.println(select);
        }
        
        prln("End editing");
        adminLogout();
      }
      
    }else {
      isAdminLogin = true;
    }
  }else{
    // not admin
    prln("is user");
    Serial.println(roles[findRole()]);
    prln("showoptionled");
    showOptionLed(roles[findRole()]);
    

    if (isAdminLogin == true){
      if (isEditMode == true) {
        if (isEditing == true){
          select = (++select)%32;
          Serial.print("select: ");Serial.println(select);
          showOptionLed(select);
        }
        
        //switch to editing
        isEditing = true;
        prln("is Editing");
      }else {
        
        //admin logout
        adminLogout();
      }
    }else {
      // admin not login before
    }
    lastRole = findRole();
  }
}
void showOptionLed(int row){
  Serial.print("turn on led with role: "); Serial.println(row);
  int lenght = sizeof(option[row]);
  for (int i = 0; i< lenght; i++){
    digitalWrite(led[i], LOW);
    if (option[row][i] == 1){
      digitalWrite(led[i], HIGH);
    }
  }
}
//void turnOnLed(int row){
//  prln("turnOnLed");
//  int lenght = sizeof(roles[row]);
//  for (int i = 0; i< lenght; i++){
//    digitalWrite(led[i], LOW);
//    if (roles[row][i] == 1){
//      digitalWrite(led[i], HIGH);
//    }
//  }
//}
void turnOffLed(){
  for (int i = 0;  i< sizeof(led); i++){
    digitalWrite(led[i], LOW);
  }
}
void blinks(){
  for (int i = 0;  i< 5; i++){
    digitalWrite(led[i], HIGH);
    delay(100);
    digitalWrite(led[i], LOW);
  }
}

int findRole(){
  for (int i=0; i< 4; i++){
    printHex(rfid.uid.uidByte,4);
    if (rfid.uid.uidByte[0] == userKeys[i][0]){
      if (compare(rfid.uid.uidByte,userKeys[i], rfid.uid.size) == true){
        return i;
      }
    }
  }
  return -1;
}

void adminLogout(){
  isAdminLogin  = false;
  isEditMode    = false;
  isEditing    = false;
  turnOffLed();
  prln("admin logout");
}

void prln(String mes){
  Serial.println(mes);
}
/**
 * Helper compare two card
 */
bool compare(byte *card1, byte *card2, byte bufferSize) {
//  for (byte i = 0; i < bufferSize; i++) {
//    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
//    Serial.print(buffer[i], DEC);
//  }
  for (byte i = 0; i < bufferSize; i++) {
      if (card1[i] != card2[i]) {
        return false;
      }
  }
  return true;
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
