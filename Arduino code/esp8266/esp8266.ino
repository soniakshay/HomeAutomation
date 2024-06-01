#ifdef ARDUINO_ARCH_ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFiMulti.h>
#endif
#include <Espalexa.h>
#include <Firebase_ESP_Client.h>

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"


// Insert Firebase project API Key
#define API_KEY "YOURKEY"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "esp8266-455eb-default-rtdb.firebaseio.com/ " 

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

Espalexa espalexa;
bool signupOK = false;


ESP8266WiFiMulti wifiMulti;

String Device_1_Name = "Light1";
String Device_2_Name = "Light2";
String Device_3_Name = "Light3";
String Device_4_Name = "Light4";
String Device_5_Name = "Light5";
String Device_6_Name = "Light6";
String Device_7_Name = "Light7";
String Device_8_Name = "Light8";
String Device_9_Name = "Light9";
String Device_10_Name = "Light10";
String Device_11_Name = "Light11";
String Device_12_Name = "Light12";
String Device_13_Name = "Light13";
String Device_14_Name = "Light14";
String Device_15_Name = "Light15";
String Device_16_Name = "Light16";

void firstLightChanged(uint8_t brightness);
void secondLightChanged(uint8_t brightness);
void thirtdLightChanged(uint8_t brightness);
void fourthLightChanged(uint8_t brightness);

void fifthLightChanged(uint8_t brightness);
void sixthLightChanged(uint8_t brightness);
void sevenLightChanged(uint8_t brightness);
void eightLightChanged(uint8_t brightness);

void nineLightChanged(uint8_t brightness);
void tenLightChanged(uint8_t brightness);
void elevenLightChanged(uint8_t brightness);
void twelveLightChanged(uint8_t brightness);

void thirteenLightChanged(uint8_t brightness);
void fourteenLightChanged(uint8_t brightness);
void fifteenLightChanged(uint8_t brightness);
void sixteenLightChanged(uint8_t brightness);



void setup(){
  Serial.begin(9600);
  conncection();

}

void conncection() {
  WiFi.persistent(false);
  wifiMulti.addAP("WIFINAME", "WIFI_PASSWORD");
  wifiMulti.addAP("WIFINAME", "WIFI_PASSWORD");
  wifiMulti.addAP("WIFINAME", "WIFI_PASSWORD");
  wifiMulti.addAP("WIFINAME", "WIFI_PASSWORD");
  
      
  Serial.print("Connecting to Wi-Fi");
  while (wifiMulti.run() != WL_CONNECTED){
    Serial.print(".");
    delay(1000);
  }
  Serial.print("Connected with IP: ");
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("Firebase Connected");
    signupOK = true;
      espalexa.addDevice(Device_1_Name,firstLightChanged); //simplest definition, default state off
      espalexa.addDevice(Device_2_Name,secondLightChanged);
      espalexa.addDevice(Device_3_Name,thirtdLightChanged); //simplest definition, default state off
      espalexa.addDevice(Device_4_Name,fourthLightChanged);

      espalexa.addDevice(Device_5_Name,fifthLightChanged); //simplest definition, default state off
      espalexa.addDevice(Device_6_Name,sixthLightChanged);
      espalexa.addDevice(Device_7_Name,sevenLightChanged); //simplest definition, default state off
      espalexa.addDevice(Device_8_Name,eightLightChanged);

      espalexa.addDevice(Device_9_Name, nineLightChanged); //simplest definition, default state off
      espalexa.addDevice(Device_10_Name,tenLightChanged);
      espalexa.addDevice(Device_11_Name,elevenLightChanged); //simplest definition, default state off
      espalexa.addDevice(Device_12_Name,twelveLightChanged);

      espalexa.addDevice(Device_13_Name,thirteenLightChanged); //simplest definition, default state off
      espalexa.addDevice(Device_14_Name,fourteenLightChanged);
      espalexa.addDevice(Device_15_Name,fifteenLightChanged); //simplest definition, default state off
      espalexa.addDevice(Device_16_Name,sixteenLightChanged);
     
      espalexa.begin();
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  config.token_status_callback = tokenStatusCallback; 
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}
void loop(){

  while (wifiMulti.run() != WL_CONNECTED){
    conncection();
  }
  if (Firebase.ready() && signupOK ){
       if(Firebase.RTDB.getJSON(&fbdo, "/isUpdated")) {
          bool value = fbdo.to<bool>();
          if(value) {
            FirebaseJson updateData;
            if(Firebase.RTDB.getJSON(&fbdo, "/lights"))  {
                updateData.add("isUpdated",false);
                Serial.println(fbdo.to<FirebaseJson>().raw());
                 Firebase.RTDB.updateNode(&fbdo, "/",&updateData);
            } else {
             Serial.println(fbdo.errorReason().c_str());
             } 
          }

        }
    Serial.flush();

  }
  espalexa.loop();
  delay(800);
}

void firstLightChanged(uint8_t brightness) {
  
  FirebaseJson updateData;
  if(brightness == 255) 
  {
      updateData.add("light1","ON");
      Firebase.RTDB.updateNode(&fbdo, "/lights",&updateData);
  }  else {
      updateData.add("light1","OFF");
      Firebase.RTDB.updateNode(&fbdo, "/lights",&updateData);
  }
 }

 void secondLightChanged(uint8_t brightness) {
  
  FirebaseJson updateData;
  if(brightness == 255) 
  {
      updateData.add("light2","ON");
      Firebase.RTDB.updateNode(&fbdo, "/lights",&updateData);
  }  else {
      updateData.add("light2","OFF");
      Firebase.RTDB.updateNode(&fbdo, "/lights",&updateData);
  }
 }


void thirtdLightChanged(uint8_t brightness)
{
  
  FirebaseJson updateData;
  if(brightness == 255) 
  {
      updateData.add("light3","ON");
      Firebase.RTDB.updateNode(&fbdo, "/lights",&updateData);
  }  else {
      updateData.add("light3","OFF");
      Firebase.RTDB.updateNode(&fbdo, "/lights",&updateData);
  }
 }



void fourthLightChanged(uint8_t brightness)
{
  
  FirebaseJson updateData;
  if(brightness == 255) 
  {
      updateData.add("light4","ON");
      Firebase.RTDB.updateNode(&fbdo, "/lights",&updateData);
  }  else {
      updateData.add("light4","OFF");
      Firebase.RTDB.updateNode(&fbdo, "/lights",&updateData);
  }
 }


void fifthLightChanged(uint8_t brightness)
{
  
  FirebaseJson updateData;
  if(brightness == 255) 
  {
      updateData.add("light5","ON");
      Firebase.RTDB.updateNode(&fbdo, "/lights",&updateData);
  }  else {
      updateData.add("light5","OFF");
      Firebase.RTDB.updateNode(&fbdo, "/lights",&updateData);
  }
 }


void sixthLightChanged(uint8_t brightness)
{
  
  FirebaseJson updateData;
  if(brightness == 255) 
  {
      updateData.add("light6","ON");
      Firebase.RTDB.updateNode(&fbdo, "/lights",&updateData);
  }  else {
      updateData.add("light6","OFF");
      Firebase.RTDB.updateNode(&fbdo, "/lights",&updateData);
  }
 }


void sevenLightChanged(uint8_t brightness)
{
  
  FirebaseJson updateData;
  if(brightness == 255) 
  {
      updateData.add("light7","ON");
      Firebase.RTDB.updateNode(&fbdo, "/lights",&updateData);
  }  else {
      updateData.add("light7","OFF");
      Firebase.RTDB.updateNode(&fbdo, "/lights",&updateData);
  }
 }



void eightLightChanged(uint8_t brightness)
{
  
  FirebaseJson updateData;
  if(brightness == 255) 
  {
      updateData.add("light8","ON");
      Firebase.RTDB.updateNode(&fbdo, "/lights",&updateData);
  }  else {
      updateData.add("light8","OFF");
      Firebase.RTDB.updateNode(&fbdo, "/lights",&updateData);
  }
 }






 void nineLightChanged(uint8_t brightness)
{
  
  FirebaseJson updateData;
  if(brightness == 255) 
  {
      updateData.add("light9","ON");
      Firebase.RTDB.updateNode(&fbdo, "/lights",&updateData);
  }  else {
      updateData.add("light9","OFF");
      Firebase.RTDB.updateNode(&fbdo, "/lights",&updateData);
  }
 }

void tenLightChanged(uint8_t brightness)
{
  
  FirebaseJson updateData;
  if(brightness == 255) 
  {
      updateData.add("light10","ON");
      Firebase.RTDB.updateNode(&fbdo, "/lights",&updateData);
  }  else {
      updateData.add("light10","OFF");
      Firebase.RTDB.updateNode(&fbdo, "/lights",&updateData);
  }
 }




 

void elevenLightChanged(uint8_t brightness)
{
  
  FirebaseJson updateData;
  if(brightness == 255) 
  {
      updateData.add("light11","ON");
      Firebase.RTDB.updateNode(&fbdo, "/lights",&updateData);
  }  else {
      updateData.add("light11","OFF");
      Firebase.RTDB.updateNode(&fbdo, "/lights",&updateData);
  }
 }



void twelveLightChanged(uint8_t brightness)
{
  
  FirebaseJson updateData;
  if(brightness == 255) 
  {
      updateData.add("light12","ON");
      Firebase.RTDB.updateNode(&fbdo, "/lights",&updateData);
  }  else {
      updateData.add("light12","OFF");
      Firebase.RTDB.updateNode(&fbdo, "/lights",&updateData);
  }
 }




 

void thirteenLightChanged(uint8_t brightness)
{
  
  FirebaseJson updateData;
  if(brightness == 255) 
  {
      updateData.add("light13","ON");
      Firebase.RTDB.updateNode(&fbdo, "/lights",&updateData);
  }  else {
      updateData.add("light13","OFF");
      Firebase.RTDB.updateNode(&fbdo, "/lights",&updateData);
  }
 }



void fourteenLightChanged(uint8_t brightness)
{
  
  FirebaseJson updateData;
  if(brightness == 255) 
  {
      updateData.add("light14","ON");
      Firebase.RTDB.updateNode(&fbdo, "/lights",&updateData);
  }  else {
      updateData.add("light14","OFF");
      Firebase.RTDB.updateNode(&fbdo, "/lights",&updateData);
  }
 }


void fifteenLightChanged(uint8_t brightness)
{
  
  FirebaseJson updateData;
  if(brightness == 255) 
  {
      updateData.add("light15","ON");
      Firebase.RTDB.updateNode(&fbdo, "/lights",&updateData);
  }  else {
      updateData.add("light15","OFF");
      Firebase.RTDB.updateNode(&fbdo, "/lights",&updateData);
  }
 }


void sixteenLightChanged(uint8_t brightness)
{
  
  FirebaseJson updateData;
  if(brightness == 255) 
  {
      updateData.add("light16","ON");
      Firebase.RTDB.updateNode(&fbdo, "/lights",&updateData);
  }  else {
      updateData.add("light16","OFF");
      Firebase.RTDB.updateNode(&fbdo, "/lights",&updateData);
  }
 }






 
