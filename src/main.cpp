#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <GyverMotor2.h>
#include <GParser.h>
#include <Servo.h>
// Определяем название и пароль точки доступа
Servo twistlocks;

const char *__SSID = "HORIZONE";
const char *__PSWD = "123456789";
const uint32_t __PORT = 49152;

const int8_t Rotate_DIR = 13;
const int8_t Rotate_PWM = 12;

const int8_t Telescop_DIR = 4;
const int8_t Telescop_PWM = 5;

const char *HELLO_MSG = "_B_SPREADER";

const uint8_t BUFFER_SIZE = 32;

static uint32_t tmr_ping = millis();
static uint32_t tmr_ping_interval = 50; // 1354;

GMotor2<DRIVER2WIRE> MOT_Rotate(Rotate_DIR, Rotate_PWM);
GMotor2<DRIVER2WIRE> MOT_Telescop(Telescop_DIR, Telescop_PWM);

WiFiUDP UDP;

void setup()
{
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.begin(__SSID, __PSWD);

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(250);
  }

  Serial.println(WiFi.gatewayIP());
  Serial.println(WiFi.localIP());

  UDP.beginPacket(WiFi.gatewayIP(), __PORT);
  UDP.printf(HELLO_MSG);
  UDP.endPacket();

  UDP.begin(__PORT);

  twistlocks.attach(14); //

  MOT_Rotate.setMinDuty(220); // мин. ШИМ
  MOT_Rotate.reverse(1);      // реверс
  MOT_Rotate.setDeadtime(5);  // deadtime
  MOT_Rotate.smoothMode(1);

  MOT_Telescop.setMinDuty(220); // мин. ШИМ
  MOT_Telescop.reverse(1);      // реверс
  MOT_Telescop.setDeadtime(5);  // deadtime
  MOT_Telescop.smoothMode(1);
}

void loop()
{
  MOT_Rotate.tick();
  MOT_Telescop.tick();

  if (WiFi.status() != WL_CONNECTED)
  {
    while (WiFi.status() != WL_CONNECTED)
    {
      Serial.print(".");
      WiFi.mode(WIFI_STA);
      WiFi.begin(__SSID, __PSWD);
      delay(250);
    }
  }

  int packetSize = UDP.parsePacket();
  if (packetSize)
  {
    Serial.print("Received packet! Size: ");
    Serial.println(packetSize);
    Serial.print("Packet: ");
    char packet[BUFFER_SIZE];
    int len = UDP.read(packet, BUFFER_SIZE);
    Serial.println(packet);
    if (len > 0)
    {
      packet[len] = '\0';
      GParser data(packet);
      int ints[data.amount()];
      data.parseInts(ints);

      switch (ints[0])
      {
      case 0:
        if (ints[1])
        {
          MOT_Rotate.setSpeed(ints[6]);
          MOT_Telescop.setSpeed(ints[7]);
          twistlocks.write(ints[8]);
        }
        break;
      }
    }
  }
}