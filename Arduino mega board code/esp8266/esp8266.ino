#ifdef ARDUINO_ARCH_ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFiMulti.h>
#endif
#include <Firebase_ESP_Client.h>
#include <time.h>
#include <EEPROM.h>

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

bool signupOK = false;

ESP8266WiFiMulti wifiMulti;

void connectWiFi();
void setupFirebaseAndDevices();
void checkSerialCommand();

void setup(){
  Serial.begin(9600);

  // Register the known access points once
  WiFi.persistent(false);
  wifiMulti.addAP("Airtel_Akshay", "Akshu@8000");
  wifiMulti.addAP("Akshay", "Akshu@8000");
  wifiMulti.addAP("Akshay", "123456789");
  wifiMulti.addAP("Kajal", "Akshu@8000");
  wifiMulti.addAP("Kajal", "123456789");
  loadSavedWifi();   
  connectWiFi();          // blocks until WiFi is up
  setupFirebaseAndDevices(); // one-time: Firebase signup

  // Configure time with NTP server
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  // Wait for time to be set
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
}


void saveWifi(const String& ssid, const String& pass) {
  EEPROM.begin(128);
  EEPROM.write(0, 0xA5);
  for (int i = 0; i < 32; i++) EEPROM.write(1 + i, i < (int)ssid.length() ? ssid[i] : 0);
  for (int i = 0; i < 64; i++) EEPROM.write(33 + i, i < (int)pass.length() ? pass[i] : 0);
  EEPROM.commit();
}

void loadSavedWifi() {
  EEPROM.begin(128);
  if (EEPROM.read(0) != 0xA5) return;
  char s[33], p[65];
  for (int i = 0; i < 32; i++) s[i] = EEPROM.read(1 + i);
  for (int i = 0; i < 64; i++) p[i] = EEPROM.read(33 + i);
  s[32] = 0; p[64] = 0;
  if (s[0]) wifiMulti.addAP(s, p);
}
// Blocks only until WiFi (station) is connected. Safe to call repeatedly —
// it does NOT touch Firebase, so calling it again on a dropped
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

// Firebase setup / signup.
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

  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
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
        saveWifi(newSsid, newPass);
        WiFi.disconnect();
        WiFi.begin(newSsid.c_str(), newPass.c_str());
        unsigned long t = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - t < 15000) {
          delay(250);
          yield();
        }
        if (WiFi.status() != WL_CONNECTED) connectWiFi();
      } else {
        Serial.println("WIFI: command missing comma - expected WIFI:ssid,password");
      }
      return;
    }
  }
}

void loop(){
  unsigned long currentMillis = millis();

  // If WiFi drops, just reconnect the radio - do NOT re-run Firebase signup.
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
  if (!signupOK && (currentMillis - lastSignupAttemptMillis >= signupRetryInterval)) {
    lastSignupAttemptMillis = currentMillis;
    Serial.println("Firebase not signed up yet - retrying...");
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

  delay(1);
}
