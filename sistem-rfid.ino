/*
 * This ESP32 code is created by esp32io.com
 *
 * This ESP32 code is released in the public domain
 *
 * For more detail (instruction and wiring diagram), visit https://esp32io.com/tutorials/esp32-rfid-nfc
 */

#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>

#define SS_PIN  5  // ESP32 pin GIOP5 
#define RST_PIN 27 // ESP32 pin GIOP27 
#define LED_MERAH 17
#define LED_KUNING 16
const String API_HOST= "https://sma1contoh.sekolahkita.net/api/";
const char WIFI_SSID[] = "Nokia 6630";
const char WIFI_PASSWORD[] = "085320059821";

String ModeAlat = "SCAN";

LiquidCrystal_I2C lcd_i2c(0x27, 16, 2); // I2C address 0x27, 16 column and 2 rows
MFRC522 rfid(SS_PIN, RST_PIN);
HTTPClient http;

void setup() {
  Serial.begin(9600);
  pinMode(LED_MERAH, OUTPUT); 
  pinMode(LED_KUNING, OUTPUT); 
  SPI.begin(); // init SPI bus
  rfid.PCD_Init(); // init MFRC522
  
  Serial.print("Menghubungkan"); 
  lcd_i2c.init(); // initialize the lcd
  lcd_i2c.backlight();
  lcd_i2c.setCursor(0, 0);      // move cursor to   (0, 1) 
  reconnectWifi();
  ModeDevice();
}

void renderScreen(String message,int x,int y){
    lcd_i2c.setCursor(y, x);      // move cursor to   (0, 1)
    lcd_i2c.print(message);
}

void reconnectWifi(){

    WiFi.begin(WIFI_SSID,WIFI_PASSWORD);
    //WiFi.mode(WIFI_STA);
    String titik;
    int titikcount = 0;
    lcd_i2c.init(); // initialize the lcd
    lcd_i2c.backlight();
    while(WiFi.status() != WL_CONNECTED){
    if(titikcount !=16){
        lcd_i2c.setCursor(0, 0);
        lcd_i2c.print("SSID ");
        lcd_i2c.print(WIFI_SSID);
        delay(500);
        titik += '.';
        Serial.print('.');
      }else{
        //lcd_i2c.print("Connecting WiFi");
        titikcount = 0;
        titik = "";
        Serial.println(' ');
        lcd_i2c.clear();
      }
    lcd_i2c.setCursor(0, 1);      // move cursor to   (0, 1)
    lcd_i2c.print(titik);
      //delay(500);
    titikcount++;
  }
    Serial.println("");
    Serial.print("Scan kartu anda..");
    Serial.println(WiFi.localIP());
    lcd_i2c.clear();
    //lcd_i2c.setCursor(0,0);      // move cursor to   (0, 1)
    //lcd_i2c.print("WiFi Connected");
}

void ModeDevice(){
  HTTPClient http;
  
  Serial.print("Request Link:");
  Serial.println(ModeAlat);
  lcd_i2c.init();
  lcd_i2c.setCursor(0,1);
  lcd_i2c.print("GETING RFID MODE");
  http.begin(String(API_HOST + "absensirfid/config"));
  
  int httpCode = http.GET();            //Send the request
  String payload = http.getString();    //Get the response payload from server

  Serial.print("Response Code:"); //200 is OK
  Serial.println(httpCode);       //Print HTTP return code

  Serial.print("Returned data from Server:");
  Serial.println(payload);    //Print request response payload
  
  if(httpCode == 200){
    DynamicJsonDocument doc(1024);
  
   // Parse JSON object
    auto error = deserializeJson(doc, payload);
    if (error) {
      Serial.print(F("deserializeJson() failed with code "));
      Serial.println(error.c_str());
      return;
    }
  
    // Decode JSON/Extract values
    String responStatus = doc["status"].as<String>();
    String responMode = doc["mode"].as<String>();
  
    Serial.println();
    Serial.print("status : ");
    Serial.println(responStatus);
    
    Serial.print("mode : ");
    Serial.println(responMode);
    lcd_i2c.clear();
    lcd_i2c.init();
    //lcd_i2c.print("System Absensi RFID");
    if (responMode == "SCAN"){
      ModeAlat = "SCAN";
      lcd_i2c.setCursor(0,1);
      lcd_i2c.print(" SCAN Your RFID");
    }else if (responMode == "ADD"){
      ModeAlat = "ADD";
      lcd_i2c.setCursor(0,1);
      lcd_i2c.print(" ADD Your RFID");
    }else{
      ModeAlat = "";
      lcd_i2c.setCursor(0,1);
      lcd_i2c.print("UNDEFINED MODE");
    }
  }else{
    Serial.println("Error in response");
  }

  http.end();
  //lcd_i2c.clear();
  delay(100);
}
void postPresensi(String rfid){
    lcd_i2c.init(); 
    lcd_i2c.clear();
    lcd_i2c.setCursor(0,1);
    lcd_i2c.print(" Loading..");
    http.begin(String(API_HOST + "absensirfid"));
    
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    
    String httpRequestData = "uid="+rfid+"";
    Serial.println(httpRequestData);
    int httpCode = http.POST(httpRequestData);
    
    if(httpCode > 0){
      if(httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY){
         
         String payload = http.getString();
         
         StaticJsonDocument<1024> doc;
         DeserializationError error = deserializeJson(doc, payload);
          
          deserializeJson(doc, payload);
            
          if (error) {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.f_str());
            return;
          }
            
          const char* pesan = doc["pesan"];
          const bool status = doc["status"];
          lcd_i2c.clear();
          lcd_i2c.init(); 
          
          if(status){
            lcd_i2c.setCursor(0, 0);      // move cursor to   (0, 1)
            lcd_i2c.print(pesan);
            digitalWrite(LED_KUNING, HIGH);
          }else{
            renderScreen(pesan,0,0);
            digitalWrite(LED_MERAH, HIGH);
          }
          
          delay(2000);
         Serial.println(pesan);
         Serial.println(httpCode);
      }else{
         Serial.printf("[HTTP] GET... code: %d\n", httpCode);
      }
    }else{
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  digitalWrite(LED_MERAH, LOW);
  digitalWrite(LED_KUNING, LOW);
  lcd_i2c.init(); 
  lcd_i2c.clear();
  lcd_i2c.setCursor(0,1);
  lcd_i2c.print(" SCAN Your RFID Card");
}

void postNewCard(String rfid){
    lcd_i2c.init(); 
    lcd_i2c.clear();
    lcd_i2c.setCursor(0,1);
    lcd_i2c.print(" Loading..");
    http.begin(String (API_HOST + "absensirfid/card"));
    
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    
    String httpRequestData = "uid="+rfid+"";
    Serial.println(httpRequestData);
    int httpCode = http.POST(httpRequestData);
    
    if(httpCode > 0){
      if(httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY){
         
         String payload = http.getString();
         
         StaticJsonDocument<1024> doc;
         DeserializationError error = deserializeJson(doc, payload);
          
          deserializeJson(doc, payload);
            
          if (error) {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.f_str());
            return;
          }
            
          const char* pesan = doc["pesan"];
          const bool status = doc["status"];
          lcd_i2c.clear();
          lcd_i2c.init(); 
          
          if(status){
            lcd_i2c.setCursor(0, 0);      // move cursor to   (0, 1)
            lcd_i2c.print(pesan);
            digitalWrite(LED_KUNING, HIGH);
          }else{
            renderScreen(pesan,0,0);
            digitalWrite(LED_MERAH, HIGH);
          }
          
          delay(2000);
         Serial.println(pesan);
         Serial.println(httpCode);
      }else{
         Serial.printf("[HTTP] GET... code: %d\n", httpCode);
      }
    }else{
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
    lcd_i2c.clear();
    lcd_i2c.setCursor(0,1);
    lcd_i2c.print(" ADD Your RFID ");
    digitalWrite(LED_MERAH, LOW);
    digitalWrite(LED_KUNING, LOW);
}

void loop() {
 if(ModeAlat == "SCAN"){
   
      if (rfid.PICC_IsNewCardPresent()){// new tag is available

        if(WiFi.status() == WL_CONNECTED){
            
          if (rfid.PICC_ReadCardSerial()) { // NUID has been readed
            MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
            //Serial.print("RFID/NFC Tag Type: ");
            //Serial.println(rfid.PICC_GetTypeName(piccType));
      
            // print UID in Serial Monitor in the hex format
            Serial.print("UID:");
            String uuidcard = "";
            for (int i = 0; i < rfid.uid.size; i++) {
              Serial.print(rfid.uid.uidByte[i] < 0x10 ? ":0" : ":");
              Serial.print(rfid.uid.uidByte[i],HEX);
              uuidcard.concat(String(rfid.uid.uidByte[i] < 0x10 ? ":0" : ":"));
              uuidcard.concat(String(rfid.uid.uidByte[i], HEX));
            }
            
            uuidcard.toUpperCase();
            
            rfid.PICC_HaltA(); // halt PICC
            rfid.PCD_StopCrypto1(); // stop encryption on PCD

            postPresensi(uuidcard);
            
            delay(1000);
            Serial.println();
            Serial.print(ModeAlat);
            Serial.print(uuidcard);
          }else{
            Serial.print("galga membaca kartu");
          }
          //lcd_i2c.clear();
        }else{
          reconnectWifi();
        }
     } 
  }else if(ModeAlat == "ADD"){
      if (rfid.PICC_IsNewCardPresent()) { // new tag is available
        if(WiFi.status() == WL_CONNECTED){
          if (rfid.PICC_ReadCardSerial()) { // NUID has been readed
            MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
      
            // print UID in Serial Monitor in the hex format
            Serial.print("UID:");
            String uuidcard = "";
            for (int i = 0; i < rfid.uid.size; i++) {
              Serial.print(rfid.uid.uidByte[i] < 0x10 ? ":0" : ":");
              Serial.print(rfid.uid.uidByte[i],HEX);
              uuidcard.concat(String(rfid.uid.uidByte[i] < 0x10 ? ":0" : ":"));
              uuidcard.concat(String(rfid.uid.uidByte[i], HEX));
            }
            uuidcard.toUpperCase();
            Serial.println();
            //Serial.print(uuidcard); // print message at (0, 1)}
            rfid.PICC_HaltA(); // halt PICC
            rfid.PCD_StopCrypto1(); // stop encryption on PCD
            postNewCard(uuidcard);
            delay(1000);
            Serial.println();
            Serial.print(ModeAlat);
            Serial.print(uuidcard);
          }
        }else{
          reconnectWifi();
        }
     } 
  }else{
     lcd_i2c.init(); 
     lcd_i2c.clear();
     lcd_i2c.setCursor(0,1);
     lcd_i2c.print("MODE NOT DEFINED");
     Serial.println();
     Serial.print("MODE NOT DEFINED");
  }
}
