#define WEBSOCKETS_LOGLEVEL     2
#include <WiFiClientSecure.h>
#include <WebSocketsServer_Generic.h>
#include <ArduinoJson.h>
#include"servo_loco.h"
#define WS_PORT 80


WebSocketsServer  webSocket = WebSocketsServer(WS_PORT);

String mpu = "";             //mpu data
String command = "";         // commands
String channel;         //channel name received from payload
String loc_com = "";
DynamicJsonDocument doc(1024);

int arr[10];

void hexdump(const void *mem, const uint32_t& len, const uint8_t& cols = 16)
{
  const uint8_t* src = (const uint8_t*) mem;
  for (uint32_t i = 0; i < len; i++)
  {
    if (i % cols == 0)
    {
    }
    src++;
  }
}

void webSocketEvent(const uint8_t& num, const WStype_t& type, uint8_t * payload, const size_t& length)
{

  switch (type)
  {
    case WStype_DISCONNECTED:
      if (arr[(int)num] == 1)
      {
        //        Serial1.print("{COM:\"S\"}");
      }

      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(arr[(int)num]);
        webSocket.sendTXT(arr[(int)num], "Connected");

        channel = (char *)payload;
        if (channel == "/MPU")
        {
          arr[(int)num] = 0;
        }
        else if (channel == "/COM" )
        {
          arr[(int)num] = 1;
        }
        else if (channel == "/SOS" )
        {
          arr[(int)num] = 2;
        }
        else if (channel == "/LOC" )
        {
          arr[(int)num] = 3;
        }
      }
      break;

    case WStype_TEXT:
      if (arr[(int)num] == 0)
      {
        mpu =  String((char*)payload);
        webSocket.sendTXT(2, mpu);
      }

      if ( arr[(int)num] == 1)
      {
        command =  String((char*)payload);
        Serial.println(command);

        deserializeJson(doc, command);
        const char* sensor = doc["COM"];
        Serial.println(sensor);
        switch (sensor[0])
        {
          case 'B':
            Serial.println("Hi from back!");
            break;
          case 'F':
            Serial.println("Hi from Front!");
            locoLoop();
            break;
          case 'R':
            Serial.println("Hi from right!");
            break;
          case 'L':
            Serial.println("Hi from Left!");
            break;
          default:
            Serial.println("Hi from def!");
            break;
        }
      }

      if ( arr[(int)num] == 3)
      {
        digitalWrite(13, HIGH);
        loc_com =  String((char*)payload);
        loc_com =  loc_com + String('*');
      }
      break;

    default:
      break;
  }
}

void setup()
{
  pinMode(12 , OUTPUT);
  pinMode(13 , OUTPUT);
  pinMode(2 , OUTPUT);
  digitalWrite(12 , LOW);
  digitalWrite(13 , LOW);
  digitalWrite(2 , LOW);
  Serial.begin(38400);
  Serial.print(F("Connecting Wifi"));
  WiFi.begin("R1-Server", "R1-Server");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(F("."));
    delay(500);
  }
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  Serial.print(F("WebSocket IP address: "));
  Serial.println(WiFi.localIP());
  servoSetup();
}

void loop()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    digitalWrite(2 , LOW);
    digitalWrite(12 , LOW);

    WiFi.begin("R1-Server", "R1-Server");
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print(F("."));
      delay(200);
    }
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
  }


  digitalWrite(2 , HIGH);
  digitalWrite(12 , HIGH);
  webSocket.loop();

  mpu = "";
  command = "";
  loc_com = "";
  delay(15);
}
