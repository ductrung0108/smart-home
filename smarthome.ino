/*
PCA: 
22 SCL
21 SDA
3v VCC
GND GND

LEDS
32
33
25

MQ
AO 17

Transistor
SIG 16

Ultrasonic
ECHO - 18
TRIG - 19

Temperature
34

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
#include <DHT.h>
const int echo = 18;  
const int trig = 19; 
#define DHTPIN 34     
#define DHTTYPE DHT11 // DHT 22 (AM2302)
 
#define WIFI_SSID "TuanAnh (2)"
#define WIFI_PASSWORD "Buituananh"
#define API_KEY "AIzaSyCH7NojcKtJIG98LKan1WxfFsDIn1bNe9A"
#define DATABASE_URL "https://smart-home-d65f6-default-rtdb.firebaseio.com" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app
#define USER_EMAIL "dctrung0108@gmail.com"
#define USER_PASSWORD "dctrung0108@gmail.com"

FirebaseData fbdo;
FirebaseData setdb;
FirebaseAuth auth;
FirebaseConfig config;
Adafruit_PWMServoDriver pca9685 = Adafruit_PWMServoDriver(0x40);

unsigned long sendDataPrevMillis = 0;
int count = 0;
uint32_t idleTimeForStream = 15000;

int winservo = 0;
int lockservo = 1;
int sweepservo = 2;
int wsprayservo = 3;

int led2 = 32;
int led3 = 33;
int led4 = 25;

int lum;
int prevgasstate;

int pwm2;

DHT dht(DHTPIN, DHTTYPE);

void setup() {

  Serial.begin(115200);

  dht.begin();

  pinMode(17, INPUT); //Gas
  pinMode(16, OUTPUT); //Water

  pinMode(trig, OUTPUT);   // chân trig sẽ phát tín hiệu
  pinMode(echo, INPUT);    // chân echo sẽ nhận tín hiệu

  pca9685.begin();
  pca9685.setPWMFreq(50); //Servos

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD); //Wifi
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

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION); //Firebase
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

void updateWaterspray() {
  if (fbdo.dataPath() == "/waterspray") {
    if (fbdo.stringData() == "true") {
      pca9685.setPWM(wsprayservo, 0, 80);
    } else if (fbdo.stringData() == "false") {
      pca9685.setPWM(wsprayservo, 0, 300);
    } else {
      return;
    } 
  }
}

void updateGas() {
  int gasstate = digitalRead(17);
  Firebase.setInt(setdb, F("/gas"), gasstate);
}

int getDistance() {
    unsigned long duration; // biến đo thời gian
    int distance;           // biến lưu khoảng cách
    
    /* Phát xung từ chân trig */
    digitalWrite(trig, 0);   // tắt chân trig
    delayMicroseconds(2);
    digitalWrite(trig, 1);   // phát xung từ chân trig
    delayMicroseconds(5);   // xung có độ dài 5 microSeconds
    digitalWrite(trig, 0);   // tắt chân trig
    
    /* Tính toán thời gian */
    // Đo độ rộng xung HIGH ở chân echo. 
    duration = pulseIn(echo, HIGH);  
    // Tính khoảng cách đến vật.
    distance = int(duration / 2 / 29.412);

    return distance;
}

void sweepServo() {
  for (int posDegrees = 0; posDegrees <= 180; posDegrees++) {
    pwm2 = map(posDegrees, 0, 180, 80, 600);
    pca9685.setPWM(sweepservo, 0, pwm2);

    int distance = getDistance();
    if (distance < 5) {
      delay(2000);
    }

    delay(50);
  }

  for (int posDegrees = 180; posDegrees >= 0; posDegrees--) {
    pwm2 = map(posDegrees, 0, 180, 80, 600);
    pca9685.setPWM(sweepservo, 0, pwm2);

    int distance = getDistance();
    if (distance < 5) {
      delay(2000); 
    }

    delay(50);
  }
}

void readTemp() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Firebase.setInt(setdb, F("/temp"), temperature);
  Firebase.setInt(setdb, F("/humidity"), humidity);
}

void loop() {

  if (Firebase.ready() && (millis() - sendDataPrevMillis > idleTimeForStream || sendDataPrevMillis == 0))
  {
    sendDataPrevMillis = millis();
  }

  if (Firebase.ready()) {
    updateGas();
    //sweepServo();
    // readTemp();
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