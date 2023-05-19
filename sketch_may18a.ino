
#define TINY_GSM_MODEM_SIM7600
#include <HardwareSerial.h>
#include <TinyGsmClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Arduino.h>
#define DEBUG true

#define mySerial Serial1
#define ONE_WIRE_BUS 4

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
int numberOfDevices;
DeviceAddress tempDeviceAddress; 

#define SIM_RESET_PIN 1
#define SIM_AT_BAUDRATE 115200
#define GSM_PIN ""
#define SIM_RX 18
#define SIM_TX 17

const char FIREBASE_HOST[] ="https://test-sim-c1cc0-default-rtdb.asia-southeast1.firebasedatabase.app/";
const String FIREBAS_AUTH = "gsDAd5Gw1LWTR46p3nj9f4v5YSVUv82HP9hpUij6";
const String FIREBASE_PATH = "/";
const int SSL_PORT = 443;

const String http_str = "AT+HTTPPARA=\"URL\",\"https://test-sim-c1cc0-default-rtdb.asia-southeast1.firebasedatabase.app/" +  FIREBAS_AUTH;

char apn[] = "v-internet";
char USER[] = "";
char PASS[] = "";

unsigned long previousMillis = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(500);
  mySerial.begin(115200,SERIAL_8N1, SIM_RX, SIM_TX);
  delay(500);
  moduleStateCheck();
  mySerial.println("AT+CCID");
  update_serial();
  delay(3000);
  mySerial.println("AT+CREG?");
  update_serial();
  delay(3000);
  mySerial.println("AT+CGATT=1");
  update_serial();
  delay(3000);
  mySerial.println("AT+CGACT=1,1");
  update_serial();
  delay(3000);
  mySerial.println("AT+CGDCONT=1,\"IP\",\"apn\"");
  update_serial();
  delay(3000);
  initDS18B20();
  delay(3000);
}
// Khởi động DS18B20
void initDS18B20(){
  sensors.begin();
  for(int i=0;i<numberOfDevices; i++){
    // Search the wire for address
    if(sensors.getAddress(tempDeviceAddress, i)){
      Serial.print("Found device ");
      Serial.print(i, DEC);
      Serial.print(" with address: ");
      printAddress(tempDeviceAddress);
      Serial.println();
    } else {
      Serial.print("Found ghost device at ");
      Serial.print(i, DEC);
      Serial.print(" but could not detect address. Check power and cabling");
    }
  }
}

void loop() {
  //Cho đọc lệnh từ serial monitor
  for(int i=0;i<numberOfDevices; i++){
    // Search the wire for address
    if(sensors.getAddress(tempDeviceAddress, i)){
      // Output the device ID
      Serial.print("Temperature for device: ");
      Serial.println(i,DEC);
      // Print the data
      float tempC = sensors.getTempC(tempDeviceAddress);
      Serial.print("Temp C: ");
      Serial.print(tempC);
    }
  }
  
  Serial.print(http_str);

  mySerial.println("AT+HTTPINIT\r\n");
  mySerial.println(http_str);
  mySerial.println("AT+HTTPACTION = 0\r\n");
  mySerial.println("AT+HTTPTERM\r\n");

  delay(5000);
}
// Hàm trả về giá trị của SIM7600 và in ra trên serial monitor
void update_serial(){
  if (mySerial.available()) {
    Serial.write(mySerial.read());
  }
}
//In device address
void printAddress(DeviceAddress deviceAddress) {
  for (uint8_t i = 0; i < 8; i++){
    if (deviceAddress[i] < 16) Serial.print("0");
      Serial.print(deviceAddress[i], HEX);
  }
}

bool moduleStateCheck()
{
    int i = 0;
    bool moduleState = false;
    for (i = 0; i < 5; i++)
    {
        String msg = String("");
        msg = mySerial.println("AT");
        if (msg.indexOf("OK") >= 0)
        {
            Serial.println("SIM7600 Module had turned on.");
            moduleState = true;
            return moduleState;
        }
        delay(1000);
    }
    return moduleState;
}

