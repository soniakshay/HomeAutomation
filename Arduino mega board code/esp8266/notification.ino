#ifdef ARDUINO_ARCH_ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFiMulti.h>
#endif
#include <Firebase_ESP_Client.h>




//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"


// Insert Firebase project API Key
#define API_KEY "APIKEY"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "esp8266-455eb-default-rtdb.firebaseio.com/" 

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;


bool signupOK = false;

ESP8266WiFiMulti wifiMulti;

// FCM API endpoint
const char* host = "fcm.googleapis.com";
const char* tokenHost = "google-access-token.onrender.com"; // First API

const int httpsPort = 443;

// FCM details
const char* projectId = "esp8266-455eb";    // Replace with your Firebase project ID
const char* deviceToken = ""; // Replace with the device token
const int pirPin = 14; 
int motionState = LOW; // Variable to store PIR state
String responseBody = "";
String accessToken = "";
String path = "/"; // Replace with the actual endpoint



void setup(){
  Serial.begin(9600);
  conncection();
  pinMode(pirPin, INPUT); // PIR sensor as input

}

void conncection() {
  WiFi.persistent(false);
  wifiMulti.addAP("Akshay", "Akshu@8000");
  wifiMulti.addAP("Akshay", "123456789");

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
void loop(){

  while (wifiMulti.run() != WL_CONNECTED){
    conncection();
  }
   motionState = digitalRead(pirPin);
  if(motionState == HIGH) {
      if (Firebase.ready() && signupOK ){
          getAccessToken();  
          Serial.print("High");
     }
    } else {
       Serial.print("LOW");
    }
 




  delay(1000);

  


}




void getDeviceToken() {

  if (Firebase.RTDB.getString(&fbdo, F("/deviceToken"))) {
      // Successfully fetched the value
       deviceToken = fbdo.to<const char *>();
       getAccessToken();

    }
}



void getAccessToken() {
   // Step 1: Get the access token
  WiFiClientSecure client;
  client.setInsecure(); // For testing, allows self-signed certificates
  if (!client.connect(tokenHost, httpsPort)) {
    Serial.println("Connection to token server failed!");
    return;
  }

  // Make the HTTP GET request

  client.println("GET " + path + " HTTP/1.1");
  client.println("Host: " + String(tokenHost));
  client.println("Connection: close");
  client.println();
  bool isBody = false;
  // Read the response
  String response = "";
  String responseBody = "";
  accessToken = "";
  while (client.connected() || client.available()) {
    if (client.available()) {
      String line = client.readStringUntil('\n');


      // Detect the end of headers (an empty line)
      if (line == "\r") {
        isBody = true; // Start reading the body
      } else if (isBody) {
       responseBody += line + "\n"; // Ensure lines are separated properly
      }
    }
  }
  client.stop();
  int startIndex = responseBody.indexOf("/*") + 2; // +2 to exclude /*
  int endIndex = responseBody.indexOf("*/");

  if (startIndex != -1 && endIndex != -1) {
    String extracted = responseBody.substring(startIndex, endIndex);
    accessToken = extracted;
  } else {
    Serial.println("Could not find /* or */");
  }



  sendFCMMessage();

}


void sendFCMMessage() {
  // Serial.print(deviceToken);
  Serial.print("deviceToken");
    Serial.print(accessToken);
  WiFiClientSecure client;
  client.setInsecure(); // For ESP8266/ESP32, allows self-signed certificates

  if (!client.connect(host, httpsPort)) {
    Serial.println("Connection to FCM failed!");
    return;
  }

  // Construct the HTTP POST request
  String url = String("/v1/projects/") + projectId + "/messages:send";
  String payload = "{\"message\": {"
                   "\"token\": \"" + String(deviceToken) + "\","
                   "\"notification\": {"
                   "\"title\": \"Danger\","
                   "\"body\": \"Danger\""
                   "}"
                   "}}";

  Serial.println("Sending FCM message...");
  
  client.println("POST " + url + " HTTP/1.1");
  client.println("Host: " + String(host));
  client.println("Authorization: Bearer " + String(accessToken));
  client.println("Content-Type: application/json");
  client.println("Content-Length: " + String(payload.length()));
  client.println();
  client.println(payload);

  // // Wait for the response
  // while (client.connected() || client.available()) {
  //   if (client.available()) {
  //     String line = client.readStringUntil('\n');
  //     Serial.println(line);
  //   }
  // }

  Serial.println("Message sent!");
}



