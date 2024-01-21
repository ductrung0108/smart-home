/*
PCA: 
22 SCL
21 SDA
3v VCC
GND GND
*/

#include <Arduino.h>
#include <WiFi.h>
#include <FirebaseESP32.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <iostream>
#include <string>
using namespace std;
 
#define WIFI_SSID "tttttrunggggg"
#define WIFI_PASSWORD "ductrung"
#define API_KEY "AIzaSyCH7NojcKtJIG98LKan1WxfFsDIn1bNe9A"
#define DATABASE_URL "https://smart-home-d65f6-default-rtdb.firebaseio.com" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app
#define USER_EMAIL "dctrung0108@gmail.com"
#define USER_PASSWORD "dctrung0108@gmail.com"

FirebaseData fbdo;
FirebaseJson json;
FirebaseAuth auth;
FirebaseConfig config;
Adafruit_PWMServoDriver pca9685 = Adafruit_PWMServoDriver(0x40);

unsigned long sendDataPrevMillis = 0;
int count = 0;
uint32_t idleTimeForStream = 15000;

int winservo = 0;
int lockservo = 1;
int led1 = 35;
int led2 = 32;
int led3 = 33;
int led4 = 25;

int lum;

void setup() {

  Serial.begin(115200);

  pca9685.begin();
  pca9685.setPWMFreq(50);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
  config.api_key = API_KEY;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  config.database_url = DATABASE_URL;
  config.token_status_callback = tokenStatusCallback; 
  Firebase.reconnectNetwork(true);
  fbdo.setBSSLBufferSize(4096 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);
  Firebase.begin(&config, &auth);
  fbdo.keepAlive(5, 5, 1);
  if (!Firebase.beginStream(fbdo, "/data"))
    Serial.printf("sream begin error, %s\n\n", fbdo.errorReason().c_str());
}

void updateWindow() {
 if (fbdo.dataPath() == "/window") {
    if (fbdo.stringData() == "true") {
      pca9685.setPWM(winservo, 0, 300);
    } else if (fbdo.stringData() == "false") {
      pca9685.setPWM(winservo, 0, 80);
    }
  } else {
    return;
  }
}

void updateLock() {
  if (fbdo.dataPath() == "/lock") {
    if (fbdo.stringData() == "true") {
      pca9685.setPWM(lockservo, 0, 600);
    } else if (fbdo.stringData() == "false") {
      pca9685.setPWM(lockservo, 0, 80);
    }
  } else {
    return;
  } 
}

void updateLight() {
  if (fbdo.dataPath() == "/luminosity") {
    lum = atoi( fbdo.stringData().c_str() );
  }
}

void updateGas() {
  
}

void loop() {
  if (Firebase.ready() && (millis() - sendDataPrevMillis > idleTimeForStream || sendDataPrevMillis == 0))
  {
    sendDataPrevMillis = millis();
  }

  if (Firebase.ready()) {
    if (!Firebase.readStream(fbdo))
      Serial.printf("sream read error, %s\n\n", fbdo.errorReason().c_str());

    if (fbdo.streamTimeout()) {
      Serial.println("stream timed out, resuming...\n");

      if (!fbdo.httpConnected())
        Serial.printf("error code: %d, reason: %s\n\n", fbdo.httpCode(), fbdo.errorReason().c_str());
    }

    if (fbdo.streamAvailable()) {
      Serial.printf("sream path, %s\nevent path, %s\ndata type, %s\nevent type, %s\nvalue, %s\n\n",
                    fbdo.streamPath().c_str(),
                    fbdo.dataPath().c_str(),
                    fbdo.dataType().c_str(),
                    fbdo.eventType().c_str(),
                    fbdo.stringData().c_str());
      updateWindow();
      updateLock();
      updateLight();

      analogWrite(led1, lum);
      analogWrite(led2, lum);
      analogWrite(led3, lum);
      analogWrite(led4, lum);
    } 

  // After calling stream.keepAlive, now we can track the server connecting status
    if (!fbdo.httpConnected())
    {
      // Server was disconnected!
    }
  }
}