
#ifdef ARDUINO_ARCH_ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#endif

#include <Firebase_ESP_Client.h>
#include <FastLED.h>
#include <ArduinoJson.h>
#include <time.h>

// =====================================================
// LED SETTINGS
// =====================================================

#define NUM_LEDS 300
#define DATA_PIN 14   // ESP8266 D5 = GPIO14

CRGB leds[NUM_LEDS];

// Shooting Star Settings
#define STAR_LENGTH 20
#define STAR_SPEED  5


// =====================================================
// NTP TIME SETTINGS
// =====================================================

const char* ntpServer = "pool.ntp.org";

// India GMT +5:30
const long gmtOffset_sec = 19800;
const int daylightOffset_sec = 0;


// =====================================================
// FIREBASE
// =====================================================

// Firebase API Key
#define API_KEY "AIzaSyCIFO0Y0rsMry0mngIrtFb5xtJISUUX-kY"

// Firebase RTDB URL
#define DATABASE_URL "esp8266-455eb-default-rtdb.firebaseio.com/"

// Firebase Objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

bool signupOK = false;


// =====================================================
// WIFI
// =====================================================

#ifdef ARDUINO_ARCH_ESP32
#else
ESP8266WiFiMulti wifiMulti;
#endif


// =====================================================
// FUNCTION DECLARATIONS
// =====================================================

void conncection();
void updateLastSeen();

void animateShootingStar(
    int startPos,
    CRGB color,
    int length,
    int speed
);

void waterFlowEffect(
    uint8_t red,
    uint8_t green,
    uint8_t blue,
    uint8_t opacity
);


// =====================================================
// SETUP
// =====================================================

void setup()
{
    Serial.begin(9600);

    // LED Setup
    FastLED.addLeds<WS2811, DATA_PIN, GRB>(
        leds,
        NUM_LEDS
    );

    FastLED.setBrightness(255);

    FastLED.clear();
    FastLED.show();


    // Connect WiFi and Firebase
    conncection();


    // NTP Time Setup
    configTime(
        gmtOffset_sec,
        daylightOffset_sec,
        ntpServer
    );

    Serial.println("Waiting for NTP time...");

    struct tm timeinfo;

    unsigned long startTime = millis();

    while (!getLocalTime(&timeinfo))
    {
        Serial.print(".");

        delay(500);

        // Don't wait forever
        if (millis() - startTime > 10000)
        {
            Serial.println();
            Serial.println("Failed to obtain time");
            break;
        }
    }

    Serial.println();
    Serial.println("Setup Complete");
}


// =====================================================
// WIFI + FIREBASE CONNECTION
// =====================================================

void conncection()
{
    WiFi.persistent(false);

#ifdef ARDUINO_ARCH_ESP32

    WiFi.mode(WIFI_STA);

    WiFi.begin(
        "Akshay",
        "Akshu@8000"
    );

#else

    wifiMulti.addAP(
        "Akshay",
        "Akshu@8000"
    );

    wifiMulti.addAP(
        "Kajal",
        "Akshu@8000"
    );

    WiFi.mode(WIFI_STA);

    WiFi.setSleepMode(WIFI_NONE_SLEEP);

#endif

    WiFi.setAutoReconnect(true);


    Serial.print("Connecting to WiFi");

#ifdef ARDUINO_ARCH_ESP32

    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(1000);
    }

#else

    while (wifiMulti.run() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(1000);
    }

#endif


    Serial.println();

    Serial.print("Connected with IP: ");

    Serial.println(
        WiFi.localIP()
    );


    // Firebase Config
    config.api_key = API_KEY;

    config.database_url = DATABASE_URL;

    config.token_status_callback =
        tokenStatusCallback;


    // Anonymous Firebase Login
    if (
        Firebase.signUp(
            &config,
            &auth,
            "",
            ""
        )
    )
    {
        Serial.println(
            "Firebase Connected"
        );

        signupOK = true;
    }
    else
    {
        Serial.printf(
            "Firebase Signup Error: %s\n",
            config.signer
                .signupError
                .message
                .c_str()
        );
    }


    Firebase.begin(
        &config,
        &auth
    );

    Firebase.reconnectWiFi(true);
}


// =====================================================
// UPDATE LAST SEEN
// =====================================================

void updateLastSeen()
{
    time_t now = time(nullptr);

    if (now > 100000)
    {
        FirebaseJson updateData;

        updateData.add(
            "lastseen",
            String(now)
        );

        if (
            Firebase.RTDB.updateNode(
                &fbdo,
                "/",
                &updateData
            )
        )
        {
            Serial.println(
                "Last seen updated"
            );
        }
        else
        {
            Serial.println(
                fbdo.errorReason()
            );
        }
    }
}


// =====================================================
// SHOOTING STAR EFFECT
// =====================================================

void animateShootingStar(
    int startPos,
    CRGB color,
    int length,
    int speed
)
{
    if (speed <= 0)
    {
        speed = 1;
    }


    for (
        int i = startPos - length;
        i <= NUM_LEDS + length;
        i++
    )
    {
        FastLED.clear();


        for (
            int j = 0;
            j < length;
            j++
        )
        {
            int pos = i + j;


            if (
                pos >= 0 &&
                pos < NUM_LEDS
            )
            {
                int brightness =
                    map(
                        j,
                        0,
                        length - 1,
                        255,
                        0
                    );


                CRGB adjustedColor = color;

                adjustedColor.fadeToBlackBy(
                    255 - brightness
                );


                leds[pos] =
                    adjustedColor;
            }
        }


        FastLED.show();


        delay(
            max(
                1,
                50 / speed
            )
        );
    }


    FastLED.clear();
    FastLED.show();
}


// =====================================================
// WATER FLOW EFFECT
// =====================================================

void waterFlowEffect(
    uint8_t red,
    uint8_t green,
    uint8_t blue,
    uint8_t opacity
)
{
    // Set brightness before show()
    FastLED.setBrightness(
        opacity
    );


    for (
        int i = 0;
        i < NUM_LEDS;
        i++
    )
    {
        leds[i] =
            CRGB(
                red,
                green,
                blue
            );


        FastLED.show();

        delay(10);
    }
}


// =====================================================
// MAIN LOOP
// =====================================================

void loop()
{

    // ================================================
    // WIFI RECONNECT
    // ================================================

#ifdef ARDUINO_ARCH_ESP32

    if (
        WiFi.status() != WL_CONNECTED
    )
    {
        Serial.println(
            "WiFi disconnected. Reconnecting..."
        );

        WiFi.reconnect();

        delay(1000);

        return;
    }

#else

    if (
        wifiMulti.run() != WL_CONNECTED
    )
    {
        Serial.println(
            "WiFi disconnected. Reconnecting..."
        );

        delay(1000);

        return;
    }

#endif



    // ================================================
    // FIREBASE
    // ================================================

    if (
        Firebase.ready() &&
        signupOK
    )
    {

        // Check update flag
        if (
            Firebase.RTDB.getBool(
                &fbdo,
                "/isStripLightUpdated"
            )
        )
        {

            bool value =
                fbdo.boolData();


            if (value)
            {

                // Get Strip Light JSON
                if (
                    Firebase.RTDB.getJSON(
                        &fbdo,
                        "/striplight"
                    )
                )
                {

                    String jsonStr =
                        fbdo
                        .to<FirebaseJson>()
                        .raw();


                    StaticJsonDocument<512>
                        doc;


                    DeserializationError error =
                        deserializeJson(
                            doc,
                            jsonStr
                        );


                    if (error)
                    {
                        Serial.print(
                            "deserializeJson() failed: "
                        );

                        Serial.println(
                            error.c_str()
                        );

                        delay(100);

                        return;
                    }


                    // Read Values
                    uint8_t red =
                        doc["red"] | 0;

                    uint8_t green =
                        doc["green"] | 0;

                    uint8_t blue =
                        doc["blue"] | 0;

                    uint8_t opacity =
                        doc["opacity"] | 255;

                    String effect =
                        doc["effect"]
                        | "waterflow";


                    Serial.println(
                        "LED Update Received"
                    );


                    Serial.print("Effect: ");

                    Serial.println(
                        effect
                    );


                    // Clear old LEDs
                    FastLED.clear();

                    FastLED.show();


                    // ====================================
                    // SHOOTING STAR
                    // ====================================

                    if (
                        effect ==
                        "shootingstar"
                    )
                    {

                        CRGB color(
                            red,
                            green,
                            blue
                        );


                        animateShootingStar(
                            0,
                            color,
                            STAR_LENGTH,
                            STAR_SPEED
                        );
                    }


                    // ====================================
                    // NORMAL / WATER FLOW
                    // ====================================

                    else
                    {

                        waterFlowEffect(
                            red,
                            green,
                            blue,
                            opacity
                        );
                    }


                    // ====================================
                    // IMPORTANT:
                    // RESET FIREBASE FLAG
                    // FOR ALL EFFECTS
                    // ====================================

                    if (
                        Firebase.RTDB.setBool(
                            &fbdo,
                            "/isStripLightUpdated",
                            false
                        )
                    )
                    {
                        Serial.println(
                            "Update flag reset"
                        );
                    }
                    else
                    {
                        Serial.println(
                            "Failed to reset flag:"
                        );

                        Serial.println(
                            fbdo.errorReason()
                        );
                    }
                }
                else
                {

                    Serial.println(
                        "Failed to get striplight data:"
                    );

                    Serial.println(
                        fbdo.errorReason()
                    );
                }
            }
        }
        else
        {

            // Only print actual Firebase errors
            // Uncomment for debugging

            /*
            Serial.println(
                fbdo.errorReason()
            );
            */
        }
    }


    // Small delay for ESP8266 watchdog
    delay(1);
}

