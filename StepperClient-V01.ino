#include <M5StickCPlus.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <DFRobot_LSM303.h>
#include <BH1750.h>

#define DATA_PORT 5003
#define ROTATE_REQUEST_PORT 5001
#define ROTATE_REPLY_PORT 5002
#define CLOCKWISE 1
#define ANTICLOCKWISE 2

DFRobot_LSM303 compass;
BH1750 lightMeter;
IPAddress SendIP(192, 168, 68, 255);
const char* ssid = "xxxxx";
const char* pass = "xxxxx";
WiFiUDP udpDataOut, udpRotateRequest, udpRotateReply;
int headingAvg;
float Lux;
int LuxInt;

void setup() {
  M5.begin(true, true, true);
  Serial.begin(115200);
  M5.Lcd.setRotation(2);
  M5.Lcd.setTextSize(3);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }
  Wire.begin();
  lightMeter.begin();
  compass.init();
  compass.enable();
  udpDataOut.begin(DATA_PORT);
  udpRotateRequest.begin(ROTATE_REQUEST_PORT);
  udpRotateReply.begin(ROTATE_REPLY_PORT);
  headingAvg = GetHeading();
  Lux = lightMeter.readLightLevel();
  LuxInt = round(Lux);
  //Serial.printf("Light reading = %d\n",LuxInt);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.printf("\n%d\n\n%d\n", headingAvg, LuxInt);
}

void loop() {

  char payload[40];
  int k;
  headingAvg = GetHeading();

  while (headingAvg < 210) {
    if (RequestRotate(CLOCKWISE, 16, 2)) {
      headingAvg = GetHeading();
      Lux = lightMeter.readLightLevel();
      LuxInt = round(Lux);
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setCursor(0, 0);
      M5.Lcd.printf("\n%d\n\n%d\n", headingAvg, LuxInt);
      Serial.printf("%d,%f\n", headingAvg, Lux);
      sendData(headingAvg, Lux);
    }
  }
  while (headingAvg > 50) {
    if (RequestRotate(ANTICLOCKWISE, 16, 2)) {
      headingAvg = GetHeading();
      Lux = lightMeter.readLightLevel();
      LuxInt = round(Lux);
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setCursor(0, 0);
      M5.Lcd.printf("\n%d\n\n%d\n", headingAvg, LuxInt);
      Serial.printf("%d,%f\n", headingAvg, Lux);
      sendData(headingAvg, Lux);
    }
  }
}

void sendData(int compassval, float Lux) {
  char payload[40];
  memset(payload, 0, 40);
  snprintf(payload, 40, "%d,%f\n", compassval, Lux);
  udpDataOut.beginPacket(SendIP, DATA_PORT);
  udpDataOut.print(payload);
  udpDataOut.endPacket();
}

int GetHeading() {
  float headings;
  int k;
  //Take an average of 10 compass readings - it seems to be crap
  headings = 0.0;
  for (k = 0; k < 10; k++) {
    compass.read();
    headings += compass.getNavigationAngle();
    delay(25);
  }
  return (round(headings / 10.0));
}

boolean RequestRotate(int Rdirection, int degs, int MaxWaitTime) {
  char rotateStr[5];
  char rotateReply[20];
  int payloadSize = 0, packetSize = 0;
  int waitTime = 0;
  snprintf(rotateStr, 5, "%d%d", Rdirection, degs);
  udpRotateRequest.beginPacket(SendIP, ROTATE_REQUEST_PORT);
  udpRotateRequest.print(rotateStr);
  udpRotateRequest.endPacket();
  //Now wait for a reply to say the servo server has done
  while (true) {
    packetSize = udpRotateReply.parsePacket();
    if (packetSize) {
      payloadSize = udpRotateReply.read(rotateReply, 20);
      if ((payloadSize >= 2) && (!strncmp("Ok", rotateReply, 2))) return true;
    }
    delay(1);
    if (waitTime++ >= MaxWaitTime * 1000) return false;
  }
}
