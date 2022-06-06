#include <M5StickCPlus.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <BH1750.h>

#define DATA_PORT 5003
#define ROTATE_REQUEST_PORT 5001
#define ROTATE_REPLY_PORT 5002
BH1750 lightMeter;
IPAddress SendIP(192, 168, 68, 255);
const char* ssid = "xxxxx";
const char* pass = "xxxxx";
WiFiUDP udpDataOut, udpRotateRequest, udpRotateReply;
float Lux;
int LuxInt;
static int Bearing;

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
  udpDataOut.begin(DATA_PORT);
  udpRotateRequest.begin(ROTATE_REQUEST_PORT);
  udpRotateReply.begin(ROTATE_REPLY_PORT);
  Bearing = 0;
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(1, 0);
  M5.Lcd.print("Ready");
}

void loop() {

  char payload[40];
  int k;
  int brightBearing ;
  float maxLux;
  M5.update();
  if (M5.BtnA.wasReleased()) {
    brightBearing = 0; maxLux = 0.0;
    M5.Lcd.fillScreen(RED);
    RequestRotate(49, 30);
    M5.Lcd.fillScreen(BLACK);
    for (Bearing = 50; Bearing < 210; Bearing++) {
      Bearing = RequestRotate(Bearing, 3);
      Lux = lightMeter.readLightLevel();
      if (Lux > maxLux) {
        maxLux = Lux;
        brightBearing = Bearing;
      }
      LuxInt = round(Lux);
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setCursor(0, 0);
      M5.Lcd.printf("\n%d\n\n%d\n", Bearing, LuxInt);
      Serial.printf("%d,%f\n", Bearing, Lux);
      sendData(Bearing, Lux);
    }
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setCursor(0, 0);
      M5.Lcd.print("Done");
      M5.Lcd.setCursor(0, 30);
      M5.Lcd.print(brightBearing);
      Serial.printf("Lightest bearing = %d degrees (Lux value was %f)\n", brightBearing, maxLux);
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


  int RequestRotate(int bearing, int MaxWaitTime) {
    char rotateReply[20];
    int payloadSize = 0, packetSize = 0, reply = 0;;
    int waitTime = 0;

    sendRequest(bearing);
    //Now wait for a reply to say the servo server has done the job
    while (reply != bearing) {
      packetSize = udpRotateReply.parsePacket();
      if (packetSize) {
        memset(rotateReply, 0, 20);
        payloadSize = udpRotateReply.read(rotateReply, 20);
        if (payloadSize == 3) reply = atoi(rotateReply);
        else sendRequest(bearing);
      }
      delay(1);
      if (waitTime++ >= MaxWaitTime * 1000) {
        Serial.printf("#Repeating request for bearing %d\n", bearing);
        sendRequest(bearing);
        waitTime = 0;
      }
    }
    return bearing;
  }

  void sendRequest(int bearing) {
    char bearingStr[10];
    memset(bearingStr, 0, 10);
    snprintf(bearingStr, 5, "%3d", bearing);
    udpRotateRequest.beginPacket(SendIP, ROTATE_REQUEST_PORT);
    udpRotateRequest.print(bearingStr);
    udpRotateRequest.endPacket();
  }
