#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "time.h"


// provides the token generation process info
#include "addons/TokenHelper.h"
// provides the RTDB payload printing info and other helper function
#include "addons/RTDBHelper.h"

//Insert netword credentials
#define WIFI_SSID "TEN WIFI"
#define WIFI_PASSWORD "PASSWIFI"

//Insert firebase project api key
#define API_KEY "AIzaSyAiIwWugxgKAX0V4TOn2NbqX2f1tG8FaK0"

//Insert Authorized Email and password
#define USER_EMAIL "khaitest@gmail.com"
#define USER_PASSWORD "123456"

//Insert RTDB URLefine the RTDB URL
#define DATABASE_URL "https://test-sim-c1cc0-default-rtdb.asia-southeast1.firebasedatabase.app/"

//Define one wire bus pin
#define ONE_WIRE_BUS 4
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
int numberOfDevices;
DeviceAddress tempDeviceAddress;

//Define Firebase Data object 
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Variable to save UID
String uid;

//Database main path (to be updated in setup with the UID)
String databasePath;

//Database child notes
String tempPath ="/temperature";
String timePath ="/time";
//Parent Node (to be updated in every loop)
String parentPath;

int timestamp;
float tempC;
FirebaseJson json;

const char* ntpSever = "pool.ntp.org";

//time variables
unsigned long sendDataPrevMillis = 0;
unsigned long timerDelay = 5000;

//Initial DS18B20

void initDS18B20() {
  sensors.begin();
  numberOfDevices = sensors.getDeviceCount();
  sensors.getAddress(tempDeviceAddress, 0);
  if (numberOfDevices == 0) {
    Serial.println("Could not find sensors");
  }
}

void initWifi(){
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting to wifi..");
  while (WiFi.status() !=WL_CONNECTED)
  {
    Serial.println(".");
    delay(1000);
  }
  Serial.println(WiFi.localIP());
  Serial.println();
}

unsigned long  getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)){
    //Serial.println("Failed to obtain time")
    return(0);
  }
  time(&now);
  return now;
}

void setup() {
  Serial.begin(115200);

  initWifi();
  initDS18B20();
  configTime(0, 0, ntpSever);
  
  //Assign the api key 
  config.api_key = API_KEY;

  //Assign the user credentials
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  //Assign the RTDB URL
  config.database_url = DATABASE_URL;

  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(1024);
  
  //Assign callback function for the long running token generation
  config.token_status_callback = tokenStatusCallback; //see addons

  //Assign the maximum retry of token generation
  config.max_token_generation_retry = 5;

  //Initialize  the library with the Firebase authen and config 
  Firebase.begin(&config, &auth);

  //Getting user UID 
  Serial.println("Getting User UID");
  while((auth.token.uid) == "") {
    Serial.println(".");
    delay(1000);
  }
  //Print user UID
  uid = auth.token.uid.c_str();
  Serial.print("User UID: ");
  Serial.println(uid);

  //Update database path 
  databasePath = "/UserData/" + uid + "/readings";
}

void loop() {
  if (Firebase.ready() && (millis() - sendDataPrevMillis > timerDelay || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();

    //Get current time stamp
    timestamp = getTime();
    Serial.print ("time: ");
    Serial.println(timestamp);

    //Get current temperature
    sensors.requestTemperatures();
    tempC = sensors.getTempC(tempDeviceAddress);
    Serial.print ("Temperature: ");
    Serial.print(tempC);
    Serial.println(" C");

    parentPath = databasePath + "/" + String(timestamp);

    json.set(tempPath.c_str(), String(tempC));
    json.set(timePath, String(timestamp));
    Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, parentPath.c_str(), &json) ? "ok" : fbdo.errorReason().c_str());
  }
}