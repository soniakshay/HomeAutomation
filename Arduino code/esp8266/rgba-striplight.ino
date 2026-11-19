#ifdef ARDUINO_ARCH_ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFiMulti.h>
#endif
#include <Firebase_ESP_Client.h>
#include <FastLED.h>
#include <ArduinoJson.h>


#define NUM_LEDS 600
#define DATA_PIN 5  // D1 GPIO5
// #define CLOCK_PIN 13

// // Define the array of leds
CRGB leds[NUM_LEDS];

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"


// Insert Firebase project API Key
#define API_KEY "AIzaSyCIFO0Y0rsMry0mngIrtFb5xtJISUUX-kY"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "esp8266-455eb-default-rtdb.firebaseio.com/"

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

bool signupOK = false;

ESP8266WiFiMulti wifiMulti;

#define STAR_LENGTH 20     // Length of the shooting star (head + tail)
#define STAR_SPEED  7  


void setup(){
  Serial.begin(9600);

  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS); 
  FastLED.setBrightness(100);
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
  wifiMulti.addAP("Kajal", "Akshu@8000");
  wifiMulti.addAP("Akshu", "Akshu@8000");
      
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

// ---- Multi-comet animation (non-blocking, millis based) ----
#define COMET_COUNT 3
struct Comet { float pos, speed, decay; int8_t dir; uint8_t len; CRGB color; };

static void addPx(int i, CRGB c, float v) {
  if (i < 0 || i >= NUM_LEDS) return;
  c.nscale8_video((uint8_t)(v * 255));
  leds[i] += c;
}

static void spawnComet(Comet &m, int maxLead) {
  m.dir   = random(2) ? 1 : -1;
  m.len   = random(8, 31);                       // tail length 8..30
  m.speed = random(60, 301);                     // LEDs per second
  m.decay = pow(0.03f, 1.0f / (m.len - 1));      // tail ends at ~3%
  m.color = CHSV(random(256), 255, 255);
  int lead = random(0, maxLead);                 // stagger start off-strip
  m.pos = m.dir > 0 ? -lead : NUM_LEDS - 1 + lead;
}

void cometAnimation(uint8_t opacity, uint32_t durationMs) {
  Comet c[COMET_COUNT];
  bool alive[COMET_COUNT];
  for (uint8_t k = 0; k < COMET_COUNT; k++) { spawnComet(c[k], 150); alive[k] = true; }
  FastLED.setBrightness(opacity ? opacity : 255);

  uint32_t endAt = millis() + durationMs, last = millis();
  bool any = true;
  while (any) {
    uint32_t now = millis();
    float dt = (now - last) / 1000.0f;
    last = now;
    FastLED.clear();                             // full redraw each frame = no flicker
    any = false;
    for (uint8_t k = 0; k < COMET_COUNT; k++) {
      if (!alive[k]) continue;
      Comet &m = c[k];
      m.pos += m.dir * m.speed * dt;
      float b = 1.0f;                            // head 100%, then decays
      for (uint8_t j = 0; j < m.len; j++, b *= m.decay) {
        float x = m.pos - m.dir * j;
        int lo = (int)floor(x);
        float f = x - lo;                        // sub-pixel blend = smooth motion
        addPx(lo, m.color, b * (1 - f));
        addPx(lo + 1, m.color, b * f);
      }
      // tail has fully left the strip -> it faded out LED by LED
      bool done = m.dir > 0 ? (m.pos - m.len > NUM_LEDS) : (m.pos + m.len < 0);
      if (done) {
        if (now < endAt) spawnComet(m, 30); else { alive[k] = false; continue; }
      }
      any = true;
    }
    FastLED.show();
    yield();
  }
}

// ---- Simple fade: all LEDs fade from current colour to selected colour, then stay on ----
CRGB fadeCur = CRGB::Black;   // colour currently shown (black at start = fade-in)

void fadeColor(uint8_t red, uint8_t green, uint8_t blue, uint8_t opacity) {
  CRGB from = fadeCur, to = CRGB(red, green, blue);
  to.nscale8_video(opacity);              // opacity folded into colour so it fades too
  FastLED.setBrightness(255);
  const uint16_t FADE_MS = 1000;          // transition time
  uint32_t t0 = millis(), e;
  while ((e = millis() - t0) < FADE_MS) {
    fill_solid(leds, NUM_LEDS, blend(from, to, e * 255 / FADE_MS));
    FastLED.show();
    yield();
  }
  fill_solid(leds, NUM_LEDS, to);
  FastLED.show();
  fadeCur = to;
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
                if(effect != "fade") fadeCur = CRGB::Black;   // other effects reset fade start
                if(effect == "fade") {
                    fadeColor(red,green,blue,opacity);
                    updateData.add("/isStripLightUpdated",false);   // run once, then stay on
                } else if(effect == "shootingstar") {
                  int startPosition = 0;
                  CRGB color = CRGB(red, green, blue); // Red color, adjust values as needed
                  animateShootingStar(startPosition, color, STAR_LENGTH, STAR_SPEED);
                } else if(effect == "comet") {
                    cometAnimation(opacity, 20000);
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