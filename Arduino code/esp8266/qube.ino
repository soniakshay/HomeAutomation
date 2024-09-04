#ifdef ARDUINO_ARCH_ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFiMulti.h>
#endif
#include <Firebase_ESP_Client.h>
#include <ArduinoJson.h>




//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"


#define DATA_PIN 4 // D2

// Insert Firebase project API Key
#define API_KEY "APIKEY"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "DATABASEURL" 

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;
bool signupOK = false;
ESP8266WiFiMulti wifiMulti;
const size_t JSON_BUFFER_SIZE = 2024;  // Adjust the buffer size as needed
StaticJsonDocument<JSON_BUFFER_SIZE> jsonBuffer;


int tiltState = 0;

void setup(){
  Serial.begin(9600);

  conncection();


}


void conncection() {
  pinMode(DATA_PIN, INPUT); 
  WiFi.persistent(false);
  wifiMulti.addAP("WIFINAME", "PASSWORD");
  wifiMulti.addAP("WIFINAME", "PASSWORD");
  wifiMulti.addAP("WIFINAME", "PASSWORD");
  wifiMulti.addAP("WIFINAME", "PASSWORD");
      
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

     }
  else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  config.token_status_callback = tokenStatusCallback; 
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}
void loop(){
   jsonBuffer.clear();
   tiltState = digitalRead(DATA_PIN);
  while (wifiMulti.run() != WL_CONNECTED){
    conncection();
  }
  if (Firebase.ready() && signupOK ){
           if(Firebase.RTDB.getJSON(&fbdo, "/cubeconfig/isActive")) {
              bool value = fbdo.to<bool>(); 
             if(value) {
               if(Firebase.RTDB.getJSON(&fbdo, "/cubeconfig/lights"))  {
                    String jsonStr1 = fbdo.to<FirebaseJson>().raw();
                    FirebaseJson updateData;
                    DeserializationError error = deserializeJson(jsonBuffer, jsonStr1);
                        if (error) {
                          Serial.print(F("deserializeJson() failed: "));
                           Serial.println(error.c_str());
                           return;
                       }


                        for (JsonPair kv : jsonBuffer.as<JsonObject>()) {
                            const char* value = kv.value().as<const char*>();
                            String concatenatedString = "lights/";
                             concatenatedString += value;
                            if(tiltState == HIGH) {
                             updateData.add(concatenatedString ,"OFF");

                            }
                            if(tiltState == LOW) {
                             updateData.add(concatenatedString ,"ON");

                            }
                             Serial.print(", Value: ");
                             Serial.println(kv.value().as<const char*>()); // Get value
                             
                         }
                        Firebase.RTDB.updateNode(&fbdo, "/",&updateData);

                       

               }

             }


           }
  }
   Serial.flush();

  delay(2000);
}



 
