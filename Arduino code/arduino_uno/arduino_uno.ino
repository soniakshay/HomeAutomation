#include <ArduinoJson.h>
#include <SoftwareSerial.h>

const int controlPin[16] = {4,5,6,7,8,9,10, 11,12,13,A0,A1,A2,A3,A4,A5}; // define pins
SoftwareSerial BTSerial(2, 3);   // RX | TX


const int maxSize = 16; // Maximum size of the array
String arr[maxSize]; // Declare the array
int arrSize = 0; // Current size of the array




int getArrayLength(String arr[]) {
  int count = 0;
  while (arr[count].length() != 0 && count < maxSize) {
    count++;
  }
  return count;
}

void arrayPushUnique(String value) {
  if (arrSize < maxSize) {
    // Check if the value already exists in the array
    bool isUnique = true;
    for (int i = 0; i < arrSize; i++) {
      if (arr[i] == value) {
        isUnique = false;
        break;
      }
    }
    // If the value is unique, add it to the array
    if (isUnique) {
      arr[arrSize] = value;
      arrSize++;
    } else {
     Serial.println("Value already exists in the array.");
    }
  } else {
    // Array is full, cannot push more elements
     Serial.println("Array is full, cannot push more elements.");
  }
}

void arrayRemove(String value) {
  int index = -1; // Initialize index to -1 (not found)
  
  // Find the index of the value in the array
  for (int i = 0; i < arrSize; i++) {
    if (arr[i] == value) {
      index = i;
      break;
    }
  }
  
  // If value is found, remove it by shifting elements
  if (index != -1) {
    for (int i = index; i < arrSize - 1; i++) {
      arr[i] = arr[i + 1];
    }
    arrSize--;
  } else {
    // Value not found in the array
    Serial.println("Value not found in the array.");
  }
}

bool isValueExists(String value) {
  for (int i = 0; i < arrSize; i++) {
    if (arr[i] == value) {
      return true; // Value exists in the array
    }
  }
  return false; // Value does not exist in the array
}


void setup() {
  Serial.begin(9600);
  BTSerial.begin(9600);  // HC-05 default speed in AT command mode
  for(int i=0; i<16; i++)
    {
     pinMode(controlPin[i], OUTPUT);// set pin as output
     digitalWrite(controlPin[i],HIGH);
    }
}


const size_t JSON_BUFFER_SIZE = 2024;  // Adjust the buffer size as needed
StaticJsonDocument<JSON_BUFFER_SIZE> jsonBuffer;
void loop() {
  // wifi serial 
    jsonBuffer.clear();

  if (Serial.available()) {
    String jsonStr1 = Serial.readStringUntil('\n');
    DeserializationError error = deserializeJson(jsonBuffer, jsonStr1);
     if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        return;
     }
    for(int i=0; i<16; i++)
    {

      
      String indexString = String(i + 1); // Adding 1 to i and converting it to a String
      String light = jsonBuffer["light" + indexString]; // 
       if(isValueExists("light" + indexString) ==  false) {
        if(light == "ON") {
          digitalWrite(controlPin[i],LOW);
        }
       if(light == "OFF") {
          digitalWrite(controlPin[i],HIGH);
       }

      }
  
    }


  }


  // bluthtooth serial 
  if (BTSerial.available())   {
    String jsonStr1 = BTSerial.readStringUntil('\n');
    DeserializationError error = deserializeJson(jsonBuffer, jsonStr1);
    if (error) {
       Serial.print(F("deserializeJson() failed: "));
       Serial.println(error.c_str());
       return;
     }
    for(int i=0; i<16; i++)
    {
      String indexString = String(i + 1); // Adding 1 to i and converting it to a String
      String light = jsonBuffer["light" + indexString]; // 
      if(light == "ON") {
        digitalWrite(controlPin[i],LOW);
        arrayPushUnique("light" + indexString);
      }
     if(light == "OFF") {
        digitalWrite(controlPin[i],HIGH);
        arrayRemove("light" + indexString);
     }
  
    }
      // Serial.write(BTSerial.read());  // Serial Monitor
  }

   if(arrSize > 0) {
    String concatenatedString = "";
    for (int k = 0; k < arrSize; k++) {
        concatenatedString += arr[k].substring(5);
        if (k < arrSize - 1) {

        concatenatedString += ","; // Add a space between elements


       }
    }
    concatenatedString += "";
    BTSerial.println(concatenatedString);
    delay(80);

   }  
  
}
