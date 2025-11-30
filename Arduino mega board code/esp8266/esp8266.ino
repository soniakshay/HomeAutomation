#ifdef ARDUINO_ARCH_ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFiMulti.h>
#endif

#include <Firebase_ESP_Client.h>
#include <time.h>

#include <ArduinoJson.h>

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 19800;
const int daylightOffset_sec = 0;

unsigned long previousMillis = 0;
const unsigned long interval = 3000;




#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define API_KEY "AIzaSyCIFO0Y0rsMry0mngIrtFb5xtJISUUX-kY"
#define DATABASE_URL "esp8266-455eb-default-rtdb.firebaseio.com/"

unsigned long wifiDisconnectedSince = 0;
const unsigned long wifiRestartTimeout = 15UL * 60UL * 1000UL;
unsigned long nextReconnectAttempt = 0;
unsigned long reconnectDelay = 2000;
const unsigned long maxReconnectDelay = 60UL * 1000UL;

// ---------- SCHEDULER VARIABLES ----------
unsigned long lastScheduleCheck = 0;
const unsigned long scheduleInterval = 1000; // check every 1 sec

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

ESP8266WiFiMulti wifiMulti;
bool signupOK = false;

void setup() {
    Serial.begin(9600);
    conncection();

    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Failed to obtain time");
        return;
    }
}

void conncection() {
    WiFi.persistent(false);
    wifiMulti.addAP("Akshay", "Akshu@8000");
    wifiMulti.addAP("Kajal", "Akshu@8000");

    WiFi.mode(WIFI_STA);
    WiFi.setSleepMode(WIFI_NONE_SLEEP);
    WiFi.setAutoReconnect(true);

    Serial.print("Connecting to Wi-Fi");
    while (wifiMulti.run() != WL_CONNECTED) {
        Serial.print(".");
        delay(1000);
    }

    Serial.print("Connected with IP: ");

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

void updateLastSeen() {
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
        time_t now = time(nullptr);
        if (now) {
            FirebaseJson updateData;
            updateData.add("lastseen", String(now));
            Firebase.RTDB.updateNode(&fbdo, "/", &updateData);
        }
    }
}

void checkWiFiReconnect() {
    if (WiFi.status() != WL_CONNECTED) {
        if (wifiDisconnectedSince == 0) {
            wifiDisconnectedSince = millis();
            reconnectDelay = 2000;
            nextReconnectAttempt = millis();
            Serial.println("WiFi Lost! starting reconnect attempts...");
        }

        unsigned long now = millis();
        if (now >= nextReconnectAttempt) {
            Serial.printf("Attempting WiFi reconnect (delay %lu ms)...\n", reconnectDelay);
            WiFi.reconnect();
            wifiMulti.run();

            reconnectDelay = min(reconnectDelay * 2, maxReconnectDelay);
            nextReconnectAttempt = now + reconnectDelay;
        }

        if (now - wifiDisconnectedSince >= wifiRestartTimeout) {
            Serial.println("WiFi disconnected for too long. Restarting...");
            ESP.restart();
        }
    } else {
        if (wifiDisconnectedSince != 0) {
            Serial.println("WiFi reconnected, clearing disconnect timer.");
        }

        wifiDisconnectedSince = 0;
        reconnectDelay = 1000;
        nextReconnectAttempt = 0;
    }
}

StaticJsonDocument<4096> jsonBuffer;

void checkAndRunSchedules() {
    if (!Firebase.ready()) return;

    if (Firebase.RTDB.getJSON(&fbdo, "/schedulers")) {

        String jsonStr = fbdo.to<FirebaseJson>().raw();

        // ---- PARSE JSON ----
        DeserializationError error = deserializeJson(jsonBuffer, jsonStr);
        if (error) {
            Serial.print("JSON Error: ");
            Serial.println(error.c_str());
            return;
        }

        JsonObject root = jsonBuffer.as<JsonObject>();

        // ---- LOOP ALL SCHEDULER KEYS ----
        for (JsonPair kv : root) {

            String scheduleId = kv.key().c_str();   // Firebase child key
            JsonObject item = kv.value().as<JsonObject>();
            // Read datetime
            String datetime = item["datetime"] | "";
            bool executed   = item["executed"] | false;

            if (executed) continue;   // skip completed schedules

            // ---- DATETIME TO EPOCH ----
            struct tm scheduleTime = {0};
            sscanf(datetime.c_str(), "%d-%d-%dT%d:%d:%d",
                   &scheduleTime.tm_year,
                   &scheduleTime.tm_mon,
                   &scheduleTime.tm_mday,
                   &scheduleTime.tm_hour,
                   &scheduleTime.tm_min,
                   &scheduleTime.tm_sec);

            scheduleTime.tm_year -= 1900;
            scheduleTime.tm_mon--;

            time_t scheduleEpoch = mktime(&scheduleTime);
            time_t nowEpoch = time(nullptr);

            if (nowEpoch >= scheduleEpoch) {

                // ---- READ LIGHTS OBJECT ----
                JsonObject lights = item["lights"].as<JsonObject>();

                for (JsonPair light : lights) {
                    String lightKey   = light.key().c_str();
                    String lightValue = light.value().as<const char*>();

                    String path = "/lights/" + lightKey;

                    Firebase.RTDB.setString(&fbdo, path, "ON");
                }

                // ---- SET EXECUTED ----
              Firebase.RTDB.setBool(&fbdo, "/schedulers/" + scheduleId + "/executed", true);
              Firebase.RTDB.setBool(&fbdo, "/isUpdated", true);
            }
        }

        // CLEAR BUFFER AFTER USE
        jsonBuffer.clear();
    }
}


void loop() {
    unsigned long currentMillis = millis();
    checkWiFiReconnect();

    if (Firebase.ready() && signupOK) {
        if (currentMillis - previousMillis >= interval) {
            previousMillis = currentMillis;
            updateLastSeen();
            checkAndRunSchedules();
        }

        if (Firebase.RTDB.getJSON(&fbdo, "/isUpdated")) {
            bool value = fbdo.to<bool>();

            if (value) {
                FirebaseJson updateData;

                if (Firebase.RTDB.getJSON(&fbdo, "/lights")) {
                    updateData.add("isUpdated", false);
                    Serial.println(fbdo.to<FirebaseJson>().raw());
                    Firebase.RTDB.updateNode(&fbdo, "/", &updateData);
                } else {
                    Serial.println(fbdo.errorReason().c_str());
                }
            }
        }
    }

    delay(1);
}

