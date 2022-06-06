#include <M5Atom.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#define SPIN_REQUEST_PORT 5001
#define SPIN_REPLY_PORT 5002

#define ROTATION_DELAY 8                 //Defines low pulse duration after stepper coil stimulation
#define HALFSTEP_PULSE_LEN 4             //Defines high pulse duration when stimulating stepper coil - Halfstep
#define CLOCKWISE true                   //Stepper rotation direction 
#define ANTICLOCKWISE false              //Stepper rotation direction

#define In1 33
#define In2 23
#define In3 19
#define In4 22
#define Photo 25
static int StepPosition = 0;
static int Bearing;
IPAddress SendIP(192, 168, 68, 255);
const char* ssid = "xxxxx";
const char* pass = "xxxxx";
WiFiUDP udpRX, udpTX;

void setup() {
  M5.begin(true, true, true);
  pinMode(In1, OUTPUT); digitalWrite(In1, LOW);
  pinMode(In2, OUTPUT); digitalWrite(In2, LOW);
  pinMode(In3, OUTPUT); digitalWrite(In3, LOW);
  pinMode(In4, OUTPUT); digitalWrite(In4, LOW);
  pinMode(Photo, INPUT);
  Serial.begin(115200);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    M5.dis.drawpix(4, 0xff0000);
    delay(100);
    M5.dis.clear();
    delay(100);
  }
  Bearing = 0;
  StepPosition = 0;
  M5.dis.drawpix(4, 0x00ff00);
  udpRX.begin(SPIN_REQUEST_PORT);
  udpTX.begin(SPIN_REPLY_PORT);
}

void loop() {
  int packetSize = 0;
  int payloadSize = 0;
  char packet[20];
  boolean SpinDirection;
  char StrBearing[50];
  int WantedBearing = 0;

  packetSize = udpRX.parsePacket();
  if (packetSize) {
    memset(packet, 0, 20);
    payloadSize = udpRX.read(packet, 10);
    if (payloadSize == 3) {
      M5.dis.drawpix(9, 0xff0000);
      WantedBearing = atoi(packet);
      if (WantedBearing > Bearing) {
        Serial.printf("Bearing=%d. Wanted=%d. Rotating clockwise\n", Bearing, WantedBearing);
        while (Bearing != WantedBearing) {
          RotateBase(CLOCKWISE);
          Bearing++;
        }
      }
      if (WantedBearing < Bearing) {
        Serial.printf("Bearing=%d. Wanted=%d. Rotating anti-clockwise\n", Bearing, WantedBearing);
        while (Bearing != WantedBearing) {
          RotateBase(ANTICLOCKWISE);
          Bearing--;
        }
      }
      M5.dis.drawpix(9, 0x000000);
    }
    else {
      Serial.printf("Invalid packet (%d) \"%s\"\n", payloadSize, packet);
    }
    Reply();
  }
}

void Reply() {
  char replystr[10];
  memset(replystr, 0, 10);
  snprintf(replystr, 5, "%3d", Bearing);
  udpTX.beginPacket(SendIP, SPIN_REPLY_PORT);
  udpTX.print(replystr);
  udpTX.endPacket();
}

void TogglePixels(int colour, int led1, int led2, int led3, int led4, int led5) {
  static int islit = 1;
  switch (islit) {
    case 1 : {
        M5.dis.drawpix(led5, 0xff000000);
        M5.dis.drawpix(led1, colour);
        break;
      }
    case 2 : {
        M5.dis.drawpix(led1, 0xff000000);
        M5.dis.drawpix(led2, colour);
        break;
      }
    case 3 : {
        M5.dis.drawpix(led2, 0xff000000);
        M5.dis.drawpix(led3, colour);
        break;
      }
    case 4 : {
        M5.dis.drawpix(led3, 0x000000);
        M5.dis.drawpix(led4, colour);
        break;
      }
    case 5 : {
        M5.dis.drawpix(led4, 0x000000);
        M5.dis.drawpix(led5, colour);
      }
  }
  if (islit == 5) islit = 1;
  else islit++;
}


void RotateBase(boolean RotateDirection) {
  int k;
  boolean state;
  state = (boolean) digitalRead(Photo);
  k = 0;
  while ((boolean) digitalRead(Photo) == state) {
    k++;
    if (RotateDirection == true) {
      TogglePixels(0x00ff00, 0, 5, 10, 15, 20);
      StepPosition--;
      if (StepPosition < 1) StepPosition = 8;
    }
    else {
      StepPosition++;
      TogglePixels(0x0000ff, 0, 5, 10, 15, 20);
      if (StepPosition > 8) StepPosition = 1;
    }
    switch (StepPosition) {
      case 1: {
          digitalWrite(In1, HIGH);
          delay(HALFSTEP_PULSE_LEN);
          digitalWrite(In1, LOW);
          delay(ROTATION_DELAY);
          break;
        }
      case 2: {
          digitalWrite(In1, HIGH);
          digitalWrite(In2, HIGH);
          delay(HALFSTEP_PULSE_LEN);
          digitalWrite(In1, LOW);
          digitalWrite(In2, LOW);
          delay(ROTATION_DELAY);
          break;
        }
      case 3: {
          digitalWrite(In2, HIGH);
          delay(HALFSTEP_PULSE_LEN);
          digitalWrite(In2, LOW);
          delay(ROTATION_DELAY);
          break;
        }
      case 4: {
          digitalWrite(In2, HIGH);
          digitalWrite(In3, HIGH);
          delay(HALFSTEP_PULSE_LEN);
          digitalWrite(In2, LOW);
          digitalWrite(In3, LOW);
          delay(ROTATION_DELAY);
          break;
        }
      case 5: {
          digitalWrite(In3, HIGH);
          delay(HALFSTEP_PULSE_LEN);
          digitalWrite(In3, LOW);
          delay(ROTATION_DELAY);
          break;
        }
      case 6: {
          digitalWrite(In3, HIGH);
          digitalWrite(In4, HIGH);
          delay(HALFSTEP_PULSE_LEN);
          digitalWrite(In3, LOW);
          digitalWrite(In4, LOW);
          delay(ROTATION_DELAY);
          break;
        }
      case 7: {
          digitalWrite(In4, HIGH);
          delay(HALFSTEP_PULSE_LEN);
          digitalWrite(In4, LOW);
          delay(ROTATION_DELAY);
          break;
        }
      case 8: {
          digitalWrite(In4, HIGH);
          digitalWrite(In1, HIGH);
          delay(HALFSTEP_PULSE_LEN);
          digitalWrite(In4, LOW);
          digitalWrite(In1, LOW);
          delay(ROTATION_DELAY);
          break;
        }
      default : {
          Serial.println("I slept with your Mom");
          break;
        }
    }
  }
}
