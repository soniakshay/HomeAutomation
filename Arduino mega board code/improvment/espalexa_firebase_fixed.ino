#ifdef ARDUINO_ARCH_ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFiMulti.h>
#endif
#include <Espalexa.h>
#include <Firebase_ESP_Client.h>
#include <time.h>

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 19800; // 5 hours * 3600 + 30 minutes * 60
const int daylightOffset_sec = 0;
unsigned long previousMillis = 0;
const unsigned long interval = 3000; // "lastseen" heartbeat interval (ms)

unsigned long lastSignupAttemptMillis = 0;
const unsigned long signupRetryInterval = 10000; // retry Firebase signup every 10s if it hasn't succeeded yet

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"


// Insert Firebase project API Key
#define API_KEY "AIzaSyCIFO0Y0rsMry0mngIrtFb5xtJISUUX-kY"

// Insert RTDB URL
#define DATABASE_URL "esp8266-455eb-default-rtdb.firebaseio.com/"

FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

Espalexa espalexa;
bool signupOK = false;
bool devicesRegistered = false; // guards against re-adding Espalexa devices on reconnect

ESP8266WiFiMulti wifiMulti;

// One Alexa device per Firebase "/LightGroups" entry (e.g. "bedroom" ->
// light5,light6,light7,light8), fetched at boot - so renaming a group or
// changing its light list in the app takes effect on the next reboot
// without touching this code.
const int MAX_GROUPS = 8;
String groupNames[MAX_GROUPS];
String groupLightsCsv[MAX_GROUPS]; // e.g. "light5,light6,light7,light8"
int groupCount = 0;

void fetchLightGroups();
void groupChanged(uint8_t slot, uint8_t brightness);

void connectWiFi();
void setupFirebaseAndDevices();
void checkSerialCommand();

void setup(){
  Serial.begin(9600);

  // Register the known access points once
  WiFi.mode(WIFI_STA);          // pure station mode - avoids AP+STA reconnect quirks
  WiFi.setSleepMode(WIFI_NONE_SLEEP); // modem sleep is why this drops/misses reconnects when other devices don't
  WiFi.persistent(false);
  wifiMulti.addAP("Airtel_Akshay", "Akshu@8000");
  wifiMulti.addAP("Akshay", "Akshu@8000");
  wifiMulti.addAP("Kajal", "Akshu@8000");

  connectWiFi();          // blocks until WiFi is up
  setupFirebaseAndDevices(); // one-time: Firebase signup + Espalexa devices

  // Configure time with NTP server
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  // Wait for time to be set
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
}

// Blocks only until WiFi (station) is connected. Safe to call repeatedly —
// it does NOT touch Firebase or Espalexa, so calling it again on a dropped
// connection just reconnects the radio, nothing else.
void connectWiFi() {
  Serial.print("Connecting to Wi-Fi");
  while (wifiMulti.run() != WL_CONNECTED) {
    // Keep watching for a RESET command from the UNO even while WiFi is
    // down - otherwise this blocking loop starves checkSerialCommand()
    // in loop() and RESET stops working whenever the network is offline.
    checkSerialCommand();
    Serial.print(".");
    delay(1000);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
}

// Runs ONLY ONCE (guarded by devicesRegistered/signupOK). This is what was
// previously happening on every reconnect and slowly leaking memory / adding
// duplicate Espalexa devices until the board became unstable.
void setupFirebaseAndDevices() {
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if (!signupOK) {
    if (Firebase.signUp(&config, &auth, "", "")) {
      Serial.println("Firebase Connected");
      signupOK = true;
    } else {
      Serial.printf("%s\n", config.signer.signupError.message.c_str());
    }
  }

  if (signupOK && !devicesRegistered) {
    fetchLightGroups();
    if (groupCount > 0) {
      for (int i = 0; i < groupCount; i++) {
        espalexa.addDevice(groupNames[i], [i](uint8_t brightness) {
          groupChanged(i, brightness);
        });
      }
      espalexa.begin();
      devicesRegistered = true;
    } else {
      Serial.println("LightGroups fetch failed or empty - will retry.");
    }
  }

  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

// Walks "/LightGroups" (e.g. {"bedroom":{"light5":"light5","light6":"light6"}})
// into groupNames[]/groupLightsCsv[]. FirebaseJson's iterator is a flat,
// depth-first walk: an OBJECT entry starts a new group, the STRING leaves
// that follow it (until the next OBJECT) are that group's light keys - which
// is exactly this schema's 2-level shape (group -> light leaves), so no
// generic tree/path tracking is needed.
void fetchLightGroups() {
  if (!Firebase.RTDB.getJSON(&fbdo, "/LightGroups")) {
    Serial.println(fbdo.errorReason().c_str());
    return;
  }
  FirebaseJson &json = fbdo.jsonObject();
  size_t len = json.iteratorBegin();
  groupCount = 0;
  for (size_t i = 0; i < len; i++) {
    int type;
    String key, value;
    json.iteratorGet(i, type, key, value);
    if (type == FirebaseJson::JSON_OBJECT) {
      if (groupCount >= MAX_GROUPS) break;
      groupNames[groupCount] = key;
      groupLightsCsv[groupCount] = "";
      groupCount++;
    } else if (groupCount > 0) {
      String &csv = groupLightsCsv[groupCount - 1];
      if (csv.length()) csv += ",";
      csv += key; // key == value == light1/light5/... in this schema
    }
  }
  json.iteratorEnd();
}

// Shared handler for every group device: flips every light in that group's
// CSV list ON/OFF in one Firebase update, same as the old per-light handlers.
void groupChanged(uint8_t slot, uint8_t brightness) {
  String state = brightness > 0 ? "ON" : "OFF";
  FirebaseJson updateData;
  updateData.add("isUpdated", true);

  String csv = groupLightsCsv[slot];
  int start = 0;
  while (start < (int)csv.length()) {
    int comma = csv.indexOf(',', start);
    if (comma == -1) comma = csv.length();
    String lightKey = csv.substring(start, comma);
    updateData.add("lights/" + lightKey, state);
    start = comma + 1;
  }
  Firebase.RTDB.updateNode(&fbdo, "/", &updateData);
}

void updateLastSeen() {
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
       time_t now = time(nullptr);
        if(now) {
          FirebaseJson updateData;
          updateData.add("lastseen",String(now));
          Firebase.RTDB.updateNode(&fbdo, "/",&updateData);
        }
    }
}

// The UNO is wired to this ESP8266's hardware Serial (TX/RX). It forwards
// plain-text commands here from its Bluetooth link:
//   "RESET"              -> reboot the ESP8266
//   "WIFI:ssid,password"  -> add/try a new WiFi network on the fly
void checkSerialCommand() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd == "RESET") {
      Serial.println("RESET command received from UNO - restarting ESP8266...");
      Serial.flush();
      delay(200);
      ESP.restart();
      return;
    }

    if (cmd.startsWith("WIFI:")) {
      String payload = cmd.substring(5); // strip "WIFI:"
      int commaIndex = payload.indexOf(',');
      if (commaIndex > 0) {
        String newSsid = payload.substring(0, commaIndex);
        String newPass = payload.substring(commaIndex + 1);
        newSsid.trim();
        newPass.trim();

        Serial.print("Adding WiFi network from Bluetooth: ");
        Serial.println(newSsid);

        wifiMulti.addAP(newSsid.c_str(), newPass.c_str());

        // Force a reconnect attempt so wifiMulti tries the new network
        // right away instead of waiting for the current one to drop.
        WiFi.disconnect();
        connectWiFi();
      } else {
        Serial.println("WIFI: command missing comma - expected WIFI:ssid,password");
      }
      return;
    }
  }
}

void loop(){
  unsigned long currentMillis = millis();

  // If WiFi drops, just reconnect the radio - do NOT re-run Firebase signup
  // or re-add Espalexa devices.
  if (wifiMulti.run() != WL_CONNECTED) {
    Serial.println("WiFi lost, reconnecting...");
    connectWiFi();
    // Firebase.reconnectWiFi(true) (set once in setupFirebaseAndDevices)
    // takes care of resuming Firebase automatically once WiFi is back.
  }

  // Check independent of WiFi/Firebase status - the UNO can ask for a
  // reset even if the network is down.
  checkSerialCommand();

  // Local WiFi/router can be connected while actual internet access is
  // still down (e.g. ISP outage). In that case Firebase.signUp() in
  // setup() fails once and signupOK never becomes true - since WiFi
  // itself never drops, connectWiFi() (which would otherwise trigger a
  // retry) never runs again either. So retry setup here on a timer until
  // it succeeds, without needing a manual power-cycle once internet is
  // back.
  if ((!signupOK || !devicesRegistered) && (currentMillis - lastSignupAttemptMillis >= signupRetryInterval)) {
    lastSignupAttemptMillis = currentMillis;
    Serial.println("Firebase not signed up / groups not loaded yet - retrying...");
    setupFirebaseAndDevices();
  }

  if (Firebase.ready() && signupOK) {
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;
      updateLastSeen();
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
    Serial.flush();
  }

  espalexa.loop();
  delay(1);
}
