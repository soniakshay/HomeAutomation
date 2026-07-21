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
    espalexa.addDevice(Device_1_Name, firstLightChanged);
    espalexa.addDevice(Device_2_Name, secondLightChanged);
    espalexa.addDevice(Device_3_Name, thirtdLightChanged);
    espalexa.addDevice(Device_4_Name, fourthLightChanged);

    espalexa.addDevice(Device_5_Name, fifthLightChanged);
    espalexa.addDevice(Device_6_Name, sixthLightChanged);
    espalexa.addDevice(Device_7_Name, sevenLightChanged);
    espalexa.addDevice(Device_8_Name, eightLightChanged);

    espalexa.addDevice(Device_9_Name, nineLightChanged);
    espalexa.addDevice(Device_10_Name, tenLightChanged);
    espalexa.addDevice(Device_11_Name, elevenLightChanged);
    espalexa.addDevice(Device_12_Name, twelveLightChanged);

    espalexa.addDevice(Device_13_Name, thirteenLightChanged);
    espalexa.addDevice(Device_14_Name, fourteenLightChanged);
    espalexa.addDevice(Device_15_Name, fifteenLightChanged);
    espalexa.addDevice(Device_16_Name, sixteenLightChanged);

    espalexa.begin();
    devicesRegistered = true;
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

  espalexa.loop();
  delay(1);
}

void firstLightChanged(uint8_t brightness) {
  FirebaseJson updateData;
  if(brightness == 255)
  {
      updateData.add("isUpdated",true);
      updateData.add("lights/light1","ON");
      Firebase.RTDB.updateNode(&fbdo, "/",&updateData);
  }  else {
      updateData.add("isUpdated",true);
      updateData.add("lights/light1","OFF");
      Firebase.RTDB.updateNode(&fbdo, "/",&updateData);
  }
 }

 void secondLightChanged(uint8_t brightness) {
 FirebaseJson updateData;
  if(brightness == 255)
  {
      updateData.add("isUpdated",true);
      updateData.add("lights/light2","ON");
      Firebase.RTDB.updateNode(&fbdo, "/",&updateData);
  }  else {
      updateData.add("isUpdated",true);
      updateData.add("lights/light2","OFF");
      Firebase.RTDB.updateNode(&fbdo, "/",&updateData);
  }
 }

void thirtdLightChanged(uint8_t brightness)
{
  FirebaseJson updateData;
  if(brightness == 255)
  {
      updateData.add("isUpdated",true);
      updateData.add("lights/light3","ON");
      Firebase.RTDB.updateNode(&fbdo, "/",&updateData);
  }  else {
      updateData.add("isUpdated",true);
      updateData.add("lights/light3","OFF");
      Firebase.RTDB.updateNode(&fbdo, "/",&updateData);
  }
 }

void fourthLightChanged(uint8_t brightness)
{
FirebaseJson updateData;
  if(brightness == 255)
  {
      updateData.add("isUpdated",true);
      updateData.add("lights/light4","ON");
      Firebase.RTDB.updateNode(&fbdo, "/",&updateData);
  }  else {
      updateData.add("isUpdated",true);
      updateData.add("lights/light4","OFF");
      Firebase.RTDB.updateNode(&fbdo, "/",&updateData);
  }
 }

void fifthLightChanged(uint8_t brightness)
{
FirebaseJson updateData;
  if(brightness == 255)
  {
      updateData.add("isUpdated",true);
      updateData.add("lights/light5","ON");
      Firebase.RTDB.updateNode(&fbdo, "/",&updateData);
  }  else {
      updateData.add("isUpdated",true);
      updateData.add("lights/light5","OFF");
      Firebase.RTDB.updateNode(&fbdo, "/",&updateData);
  }
 }

void sixthLightChanged(uint8_t brightness)
{
  FirebaseJson updateData;
  if(brightness == 255)
  {
      updateData.add("isUpdated",true);
      updateData.add("lights/light6","ON");
      Firebase.RTDB.updateNode(&fbdo, "/",&updateData);
  }  else {
      updateData.add("isUpdated",true);
      updateData.add("lights/light6","OFF");
      Firebase.RTDB.updateNode(&fbdo, "/",&updateData);
  }
 }

void sevenLightChanged(uint8_t brightness)
{
  FirebaseJson updateData;
  if(brightness == 255)
  {
      updateData.add("isUpdated",true);
      updateData.add("lights/light7","ON");
      Firebase.RTDB.updateNode(&fbdo, "/",&updateData);
  }  else {
      updateData.add("isUpdated",true);
      updateData.add("lights/light7","OFF");
      Firebase.RTDB.updateNode(&fbdo, "/",&updateData);
  }
 }

void eightLightChanged(uint8_t brightness)
{
FirebaseJson updateData;
  if(brightness == 255)
  {
      updateData.add("isUpdated",true);
      updateData.add("lights/light8","ON");
      Firebase.RTDB.updateNode(&fbdo, "/",&updateData);
  }  else {
      updateData.add("isUpdated",true);
      updateData.add("lights/light8","OFF");
      Firebase.RTDB.updateNode(&fbdo, "/",&updateData);
  }
 }

 void nineLightChanged(uint8_t brightness)
{
  FirebaseJson updateData;
  if(brightness == 255)
  {
      updateData.add("isUpdated",true);
      updateData.add("lights/light9","ON");
      Firebase.RTDB.updateNode(&fbdo, "/",&updateData);
  }  else {
      updateData.add("isUpdated",true);
      updateData.add("lights/light9","OFF");
      Firebase.RTDB.updateNode(&fbdo, "/",&updateData);
  }
 }

void tenLightChanged(uint8_t brightness)
{
FirebaseJson updateData;
  if(brightness == 255)
  {
      updateData.add("isUpdated",true);
      updateData.add("lights/light10","ON");
      Firebase.RTDB.updateNode(&fbdo, "/",&updateData);
  }  else {
      updateData.add("isUpdated",true);
      updateData.add("lights/light10","OFF");
      Firebase.RTDB.updateNode(&fbdo, "/",&updateData);
  }
 }

void elevenLightChanged(uint8_t brightness)
{
  FirebaseJson updateData;
  if(brightness == 255)
  {
      updateData.add("isUpdated",true);
      updateData.add("lights/light11","ON");
      Firebase.RTDB.updateNode(&fbdo, "/",&updateData);
  }  else {
      updateData.add("isUpdated",true);
      updateData.add("lights/light11","OFF");
      Firebase.RTDB.updateNode(&fbdo, "/",&updateData);
  }
 }

void twelveLightChanged(uint8_t brightness)
{
FirebaseJson updateData;
  if(brightness == 255)
  {
      updateData.add("isUpdated",true);
      updateData.add("lights/light12","ON");
      Firebase.RTDB.updateNode(&fbdo, "/",&updateData);
  }  else {
      updateData.add("isUpdated",true);
      updateData.add("lights/light12","OFF");
      Firebase.RTDB.updateNode(&fbdo, "/",&updateData);
  }
 }

void thirteenLightChanged(uint8_t brightness)
{
FirebaseJson updateData;
  if(brightness == 255)
  {
      updateData.add("isUpdated",true);
      updateData.add("lights/light13","ON");
      Firebase.RTDB.updateNode(&fbdo, "/",&updateData);
  }  else {
      updateData.add("isUpdated",true);
      updateData.add("lights/light13","OFF");
      Firebase.RTDB.updateNode(&fbdo, "/",&updateData);
  }
 }

void fourteenLightChanged(uint8_t brightness)
{
FirebaseJson updateData;
  if(brightness == 255)
  {
      updateData.add("isUpdated",true);
      updateData.add("lights/light14","ON");
      Firebase.RTDB.updateNode(&fbdo, "/",&updateData);
  }  else {
      updateData.add("isUpdated",true);
      updateData.add("lights/light14","OFF");
      Firebase.RTDB.updateNode(&fbdo, "/",&updateData);
  }
 }

void fifteenLightChanged(uint8_t brightness)
{
FirebaseJson updateData;
  if(brightness == 255)
  {
      updateData.add("isUpdated",true);
      updateData.add("lights/light15","ON");
      Firebase.RTDB.updateNode(&fbdo, "/",&updateData);
  }  else {
      updateData.add("isUpdated",true);
      updateData.add("lights/light15","OFF");
      Firebase.RTDB.updateNode(&fbdo, "/",&updateData);
  }
 }

void sixteenLightChanged(uint8_t brightness)
{
FirebaseJson updateData;
  if(brightness == 255)
  {
      updateData.add("isUpdated",true);
      updateData.add("lights/light16","ON");
      Firebase.RTDB.updateNode(&fbdo, "/",&updateData);
  }  else {
      updateData.add("isUpdated",true);
      updateData.add("lights/light16","OFF");
      Firebase.RTDB.updateNode(&fbdo, "/",&updateData);
  }
 }
