#ifdef ARDUINO_ARCH_ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFiMulti.h>
#endif
#include <Firebase_ESP_Client.h>
#include <FastLED.h>
#include <ArduinoJson.h>
#include <time.h>

// ---------------- Last Seen Configuration ----------------
const char* ntpServer = "pool.ntp.org";

const long gmtOffset_sec = 19800;  // India GMT +5:30
const int daylightOffset_sec = 0;

unsigned long previousLastSeenMillis = 0;
const unsigned long lastSeenInterval = 30000; // 30 seconds

// ---------------- LED config ----------------
#define MAX_LEDS      600   // array size (hardware / RAM max)
#define DEFAULT_LEDS  600   // Firebase ma numLeds na hoy tyare aa vaparse
#define DATA_PIN      5     // D1 GPIO5

CRGB leds[MAX_LEDS];
int numLeds = DEFAULT_LEDS; // runtime ma Firebase mathi badlay

// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert Firebase project API Key
#define API_KEY "AIzaSyCIFO0Y0rsMry0mngIrtFb5xtJISUUX-kY"

// Insert RTDB URL
#define DATABASE_URL "esp8266-455eb-default-rtdb.firebaseio.com/"

// Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

bool signupOK = false;

ESP8266WiFiMulti wifiMulti;

#define STAR_LENGTH 20   // Length of the shooting star (head + tail)
#define STAR_SPEED  7

// Fade effect ma current colour (black at start = fade-in)
CRGB fadeCur = CRGB::Black;

// ---------------- LED count helper ----------------
void applyLedCount(int n) {
  if (n < 1) n = DEFAULT_LEDS;
  if (n > MAX_LEDS) n = MAX_LEDS;
  if (n == numLeds) return;

  // count badlay tyare badha LEDs band kari do (ghate to vadhela LEDs na rahe)
  fill_solid(leds, MAX_LEDS, CRGB::Black);
  FastLED.show();
  numLeds = n;
  fadeCur = CRGB::Black;
  Serial.printf("LED count set to %d\n", numLeds);
}

// ---------------- Wi-Fi + Firebase ----------------
void conncection() {
  WiFi.persistent(false);
  wifiMulti.addAP("Kajal", "Akshu@8000");
  wifiMulti.addAP("Akshu", "Akshu@8000");

  Serial.print("Connecting to Wi-Fi");
  while (wifiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Firebase Connected");
    signupOK = true;
  } else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void setup() {
  Serial.begin(9600);

  // Hamesha MAX_LEDS register karo; numLeds thi effective length control thay chhe
  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, MAX_LEDS);
  FastLED.setBrightness(100);
  for (int i = 0; i < numLeds; i++) {
    leds[i] = CRGB(96, 96, 96);
    FastLED.show();
    FastLED.setBrightness(255);
    delay(10);
  }
  conncection();
   // Configure NTP time
  configTime(19800, 0, "pool.ntp.org", "time.google.com");

  Serial.println("NTP Time Configured");
}


bool isNtpTimeReady() {
  time_t now = time(nullptr);

  Serial.print("Current Timestamp: ");
  Serial.println((unsigned long)now);

  return now > 1000000000;
}


// ---------------- Update Last Seen ----------------
void updateLastSeen() {
  time_t now = time(nullptr);

  if (now > 1000000000) {
    String timestamp = String((unsigned long)now);

    if (Firebase.RTDB.setString(
          &fbdo,
          "/striplightStatus/lastSeen",
          timestamp)) {

      Serial.println("Last Seen Updated: " + timestamp);

    } else {
      Serial.println("Last Seen Error: " + fbdo.errorReason());
    }

  } else {
    Serial.println("Waiting for NTP Time...");
  }
}
// ---------------- Shooting star ----------------
void animateShootingStar(int startPos, CRGB color, int length, int speed) {
  for (int i = startPos - length; i <= numLeds; i++) {
    FastLED.clear();

    for (int j = 0; j < length; j++) {
      int pos = i + j;
      if (pos >= 0 && pos < numLeds) {
        int brightness = map(j, 0, length - 1, 255, 0);
        CRGB adjustedColor = color;
        adjustedColor.fadeToBlackBy(255 - brightness);
        leds[pos] = adjustedColor;
      }
    }

    FastLED.show();
    delay(50 / speed);
  }
}

// ---------------- Water flow ----------------
void waterFlowEffect(uint8_t red, uint8_t green, uint8_t blue, uint8_t opacity) {
  for (int i = 0; i < numLeds; i++) {
    leds[i] = CRGB(red, green, blue);
    FastLED.show();
    FastLED.setBrightness(opacity);
    delay(10);
  }
}

// ---------------- Sweep 1: left -> right, then erase right -> left ----------------
void sweepAnimation(uint8_t red, uint8_t green, uint8_t blue, uint8_t opacity) {
  for (int i = 0; i < numLeds; i++) {
    leds[i] = CRGB(red, green, blue);
    FastLED.show();
    FastLED.setBrightness(opacity);
    delay(10);
  }

  for (int i = numLeds - 1; i >= 0; i--) {
    leds[i] = CRGB(0, 0, 0);
    FastLED.show();
    FastLED.setBrightness(opacity);
    delay(5);
  }
}

// ---------------- Sweep 2: center -> both sides, then erase ----------------
void sweepAnimation1(uint8_t red, uint8_t green, uint8_t blue, uint8_t opacity) {
  int mid = numLeds / 2;
  int steps = (numLeds + 1) / 2;

  for (int i = 0; i < steps; i++) {
    int right = mid + i;
    int left  = mid - 1 - i;
    if (right < numLeds) leds[right] = CRGB(red, green, blue);
    if (left >= 0)       leds[left]  = CRGB(red, green, blue);
    FastLED.show();
    FastLED.setBrightness(opacity);
    delay(10);
  }

  for (int i = 0; i < steps; i++) {
    int right = mid + i;
    int left  = mid - 1 - i;
    if (right < numLeds) leds[right] = CRGB(0, 0, 0);
    if (left >= 0)       leds[left]  = CRGB(0, 0, 0);
    FastLED.show();
    FastLED.setBrightness(opacity);
    delay(10);
  }
}

// ---------------- Sweep 3: both ends -> center, then erase from center ----------------
void sweepAnimation2(uint8_t red, uint8_t green, uint8_t blue, uint8_t opacity) {
  int mid = numLeds / 2;
  int steps = (numLeds + 1) / 2;

  for (int i = 0; i < steps; i++) {
    int start = i;
    int end   = numLeds - 1 - i;
    leds[start] = CRGB(red, green, blue);
    leds[end]   = CRGB(red, green, blue);
    FastLED.show();
    FastLED.setBrightness(opacity);
    delay(10);
  }

  for (int i = 0; i < steps; i++) {
    int right = mid + i;
    int left  = mid - 1 - i;
    if (right < numLeds) leds[right] = CRGB(0, 0, 0);
    if (left >= 0)       leds[left]  = CRGB(0, 0, 0);
    FastLED.show();
    FastLED.setBrightness(opacity);
    delay(10);
  }
}

// ---------------- Multi-comet animation (non-blocking, millis based) ----------------
// ---------------- Multi-comet animation ----------------

#define COMET_COUNT 5

struct Comet {
  float pos;
  float speed;
  float decay;

  int8_t dir;
  uint8_t len;

  CRGB color;

  bool alive;
  unsigned long nextStart;
};

// IMPORTANT: Explicit function prototypes
void spawnComet(Comet &m);
void scheduleComet(Comet &m);

Comet comets[COMET_COUNT];

// Random comet start

void spawnComet(Comet &m) {

  m.dir = random(2) ? 1 : -1;

  m.len = random(15, 40);

  m.speed = random(100, 251);

  m.decay = pow(0.025f, 1.0f / (m.len - 1));

  m.color = CHSV(random(256), 255, 255);

  // Start outside strip
  if (m.dir == 1) {
    m.pos = -m.len;
  } else {
    m.pos = numLeds + m.len;
  }

  m.alive = true;
}


// Schedule next comet
void scheduleComet(Comet &m) {

  m.alive = false;

  m.nextStart = millis() + random(500, 3500);
}

// Add pixel with fractional brightness
void addCometPixel(int index, CRGB color, float brightness) {

  if (index < 0 || index >= numLeds) return;

  CRGB c = color;

  c.nscale8_video((uint8_t)(brightness * 255));

  leds[index] += c;
}

// Random shooting comet animation
void cometAnimation(uint8_t opacity, uint32_t durationMs) {

  unsigned long startTime = millis();

  unsigned long lastFrame = millis();

  for (int i = 0; i < COMET_COUNT; i++) {

    comets[i].alive = false;

    comets[i].nextStart = millis() + random(0, 4000);
  }

  FastLED.setBrightness(opacity);

  while (millis() - startTime < durationMs) {

    unsigned long now = millis();

    float dt = (now - lastFrame) / 1000.0f;

    lastFrame = now;

    FastLED.clear();

    for (int k = 0; k < COMET_COUNT; k++) {

      Comet &m = comets[k];

      // Start comet at random time
      if (!m.alive) {

        if (now >= m.nextStart) {
          spawnComet(m);
        }

        continue;
      }

      // Move comet
      m.pos += m.dir * m.speed * dt;

      // Draw head and fading tail
      float brightness = 1.0f;

      for (int j = 0; j < m.len; j++) {

        float x = m.pos - (m.dir * j);

        int lo = floor(x);

        float fraction = x - lo;

        addCometPixel(lo, m.color,
                      brightness * (1.0f - fraction));

        addCometPixel(lo + 1, m.color,
                      brightness * fraction);

        brightness *= m.decay;
      }

      // Check comet completed
      bool finished =
        (m.dir == 1 && m.pos - m.len > numLeds) ||
        (m.dir == -1 && m.pos + m.len < 0);

      if (finished) {
        scheduleComet(m);
      }
    }

    FastLED.show();

    yield();

    delay(1);
  }

  FastLED.clear();
  FastLED.show();
}


// ---------------- Simple fade: current colour -> selected colour, then stay on ----------------
void fadeColor(uint8_t red, uint8_t green, uint8_t blue, uint8_t opacity) {
  CRGB from = fadeCur, to = CRGB(red, green, blue);
  to.nscale8_video(opacity);              // opacity folded into colour so it fades too
  FastLED.setBrightness(255);
  const uint16_t FADE_MS = 1000;          // transition time
  uint32_t t0 = millis(), e;
  while ((e = millis() - t0) < FADE_MS) {
    fill_solid(leds, numLeds, blend(from, to, e * 255 / FADE_MS));
    FastLED.show();
    yield();
  }
  fill_solid(leds, numLeds, to);
  FastLED.show();
  fadeCur = to;
}

// ---------------- Main loop ----------------
void loop() {
  while (wifiMulti.run() != WL_CONNECTED) {
    conncection();
  }
  if (Firebase.ready() && signupOK) {
    // Update Last Seen every 30 seconds
      unsigned long currentMillis = millis();

      if (currentMillis - previousLastSeenMillis >= lastSeenInterval) {

        previousLastSeenMillis = currentMillis;

        updateLastSeen();
      }
    if (Firebase.RTDB.getJSON(&fbdo, "/isStripLightUpdated")) {
      bool value = fbdo.to<bool>();
      if (value) {
        FirebaseJson updateData;
        if (Firebase.RTDB.getJSON(&fbdo, "/striplight")) {

          String jsonStr = fbdo.to<FirebaseJson>().raw();
          StaticJsonDocument<256> doc;
          DeserializationError error = deserializeJson(doc, jsonStr);
          if (error) {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.c_str());
            return;
          }

          // Dynamic LED count (key na hoy to default 600)
          int n = doc["numLeds"] | DEFAULT_LEDS;
          applyLedCount(n);

          FastLED.clear();

          uint8_t red     = doc["red"];
          uint8_t green   = doc["green"];
          uint8_t blue    = doc["blue"];
          uint8_t opacity = doc["opacity"];
          String effect   = doc["effect"];

          if (effect != "fade") fadeCur = CRGB::Black;   // other effects reset fade start

          if (effect == "fade") {
            fadeColor(red, green, blue, opacity);
            updateData.add("/isStripLightUpdated", false);   // run once, then stay on
          } else if (effect == "shootingstar") {
            int startPosition = 0;
            CRGB color = CRGB(red, green, blue);
            animateShootingStar(startPosition, color, STAR_LENGTH, STAR_SPEED);
          } else if (effect == "comet") {
            cometAnimation(opacity, 20000);
          } else if (effect == "sweepAnimation") {
            sweepAnimation(red, green, blue, opacity);
          } else if (effect == "sweepAnimation1") {
            sweepAnimation1(red, green, blue, opacity);
          } else if (effect == "sweepAnimation2") {
            sweepAnimation2(red, green, blue, opacity);
          } else {
            waterFlowEffect(red, green, blue, opacity);
            updateData.add("/isStripLightUpdated", false);
          }

          Firebase.RTDB.updateNode(&fbdo, "/", &updateData);
        } else {
          Serial.println(fbdo.errorReason().c_str());
        }
      }
    }
    Serial.flush();
  }
  delay(1);
}