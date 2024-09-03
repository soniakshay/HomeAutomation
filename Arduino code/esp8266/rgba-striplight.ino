#ifdef ARDUINO_ARCH_ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFiMulti.h>
#endif
#include <Firebase_ESP_Client.h>
#include <FastLED.h>
#include <ArduinoJson.h>

// // How many leds in your strip?
 #define NUM_LEDS 900

// // For led chips like WS2812, which have a data line, ground, and power, you just
// // need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// // ground, and power), like the LPD8806 define both DATA_PIN and CLOCK_PIN
// // Clock pin only needed for SPI based chipsets when not using hardware SPI
 #define DATA_PIN 14 // D5 pin
// #define CLOCK_PIN 13

// // Define the array of leds
CRGB leds[NUM_LEDS];

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"


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

#define STAR_LENGTH 20     // Length of the shooting star (head + tail)
#define STAR_SPEED  5  


void setup(){
  Serial.begin(9600);
  FastLED.addLeds<WS2811, DATA_PIN, GRB>(leds, NUM_LEDS);
  for(int i=0;i<NUM_LEDS;i++) {
    leds[i] = CRGB(96,96,96);
    FastLED.show();
    FastLED.setBrightness(255);
    delay(10);
  }
  conncection();
 
}

void conncection() {
 
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
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  config.token_status_callback = tokenStatusCallback; 
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}


// Shooting star function
void animateShootingStar(int startPos, CRGB color, int length, int speed) {
    // Move the shooting star from the start position to the end of the strip
    for (int i = startPos - length; i <= NUM_LEDS; i++) {
        // Clear the strip
        FastLED.clear();

        // Draw the shooting star
        for (int j = 0; j < length; j++) {
            int pos = i + j;
            if (pos >= 0 && pos < NUM_LEDS) {
                // Calculate brightness based on position in the shooting star
                int brightness = map(j, 0, length - 1, 255, 0);
                // Set the color using the calculated brightness and provided color
                CRGB adjustedColor = color;
                adjustedColor.fadeToBlackBy(255 - brightness);
                leds[pos] = adjustedColor;
            }
        }

        // Show the LEDs
        FastLED.show();

        // Delay to control the speed of the shooting star
        delay(50 / speed);
    }
}


void waterFlowEffect(uint8_t red, uint8_t green, uint8_t blue,uint8_t opacity) {
  
    for(int i=0;i<NUM_LEDS;i++) {
    leds[i] = CRGB(red,green,blue);
    FastLED.show();
    FastLED.setBrightness(opacity);
    delay(10);
  }
                
}


void sweepAnimation(uint8_t red, uint8_t green, uint8_t blue,uint8_t opacity) {

  for(int i=0;i<NUM_LEDS;i++) {
    leds[i] = CRGB(red,green,blue);
    FastLED.show();
    FastLED.setBrightness(opacity);
    delay(10);
  }
  
   for(int i=NUM_LEDS;i > 0;i--) {
    leds[i] = CRGB(0,0,0);
    FastLED.show();
    FastLED.setBrightness(opacity);
    delay(5);
  }
 
}


void sweepAnimation1(uint8_t red, uint8_t green, uint8_t blue,uint8_t opacity) {

  for(int i=0;i< (NUM_LEDS/ 2);i++) {

      int start  = (NUM_LEDS/ 2) + i;
      int end  = (NUM_LEDS/ 2) - i;
     leds[start] = CRGB(red,green,blue);
      leds[end] = CRGB(red,green,blue);
      FastLED.show();
     FastLED.setBrightness(opacity);
     delay(10);
  }


  for(int i=0;i< (NUM_LEDS/ 2);i++) {

      int start  = (NUM_LEDS/ 2) + i;
      int end  = (NUM_LEDS/ 2) - i;
     leds[start] = CRGB(0,0,0);
      leds[end] = CRGB(0,0,0);
      FastLED.show();
     FastLED.setBrightness(opacity);
     delay(10);
  }  
 
}


void sweepAnimation2(uint8_t red, uint8_t green, uint8_t blue,uint8_t opacity) {

  for(int i=0;i< (NUM_LEDS / 2);i++) {

      int start = i + 1;
      int end  = NUM_LEDS - i;
      leds[start] = CRGB(red,green,blue);
      leds[end] = CRGB(red,green,blue);
      FastLED.show();
      FastLED.setBrightness(opacity);
      delay(10);
  }


  for(int i=0;i< (NUM_LEDS/ 2);i++) {

    int start  = (NUM_LEDS/ 2) + i;
    int end  = (NUM_LEDS/ 2) - i;
    leds[start] = CRGB(0,0,0);
    leds[end] = CRGB(0,0,0);
    FastLED.show();
    FastLED.setBrightness(opacity);
     delay(10);
  }  

}

void loop(){

  while (wifiMulti.run() != WL_CONNECTED){
    conncection();
  }
  if (Firebase.ready() && signupOK ){
       if(Firebase.RTDB.getJSON(&fbdo, "/isStripLightUpdated")) {
          bool value = fbdo.to<bool>();
          if(value) {
            FirebaseJson updateData;
            if(Firebase.RTDB.getJSON(&fbdo, "/striplight"))  {
               
                  String jsonStr = fbdo.to<FirebaseJson>().raw();
                  StaticJsonDocument<200> doc;
                  DeserializationError error = deserializeJson(doc, jsonStr);
                  if (error) {
                      Serial.print(F("deserializeJson() failed: "));
                      Serial.println(error.c_str());
                      return;
                  }
                
                
                 FastLED.clear();
                

                uint8_t red =  doc["red"];
                uint8_t green =  doc["green"];
                uint8_t blue  = doc["blue"];
                uint8_t opacity  = doc["opacity"];
                String effect  =  doc["effect"];
                if(effect == "shootingstar") {
                  int startPosition = 0;
                  CRGB color = CRGB(red, green, blue); // Red color, adjust values as needed
                  animateShootingStar(startPosition, color, STAR_LENGTH, STAR_SPEED);
                } else if(effect == "sweepAnimation") {
                    sweepAnimation(red,green,blue,opacity);
                } else if(effect == "sweepAnimation1") {
                      sweepAnimation1(red,green,blue,opacity);
                }else if(effect == "sweepAnimation2") {
                     sweepAnimation2(red,green,blue,opacity);
                  
              } else {
                  waterFlowEffect(red,green,blue,opacity);
                  updateData.add("/isStripLightUpdated",false);

                }
              

              Firebase.RTDB.updateNode(&fbdo, "/",&updateData);
            } else {
             Serial.println(fbdo.errorReason().c_str());
             } 
          }

        }
    Serial.flush();

  }
  delay(1);
}

