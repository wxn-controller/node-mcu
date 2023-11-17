#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>
#include "EmonLib.h"

const char *ssid = "LabControle";
const char *password = "controle@2023!";

WebSocketsServer webSocket = WebSocketsServer(81);
EnergyMonitor SCT013;

const int animationDelay = 50;
const bool useSeparatorAnimation = true;
const int wifiConnectionDelay = 500;
const int websocketDelay = 1;
const int pinRelay = 4;

int relayState = LOW;
int pinSCT = A0;
int tension = 220;
int eletricPower;

void turnONRelay() {
  digitalWrite(pinRelay, HIGH);
  relayState = HIGH;
}

void turnOFFRelay() {
  digitalWrite(pinRelay, LOW);
  relayState = LOW;
}

void useSeparator() {
  if (useSeparatorAnimation) {
    delay(animationDelay);

    Serial.print(".");
    delay(animationDelay);
    Serial.print(".");
    delay(animationDelay);
    Serial.println(".");
    delay(animationDelay);
  } else {
    Serial.print("...");
  }
}

void measure() {
  double Irms = SCT013.calcIrms(1480);  // Calculate the value of the Current
  eletricPower = Irms * tension;        // Calcalate the value of the eletricPower

  // Create a string to jSON and send the datas to the websocket client
  String data = "{\"eletricCurrent\":" + String(Irms) + ", \"eletricPower\":" + String(eletricPower) + "}";
  String message = "{\"relayState\":" + String(relayState) + ",\"data\":" + data + "}";

  webSocket.broadcastTXT(message);

  Serial.println("Corrente = " + String(Irms) + " A");
  Serial.println("Potência = " + String(eletricPower) + " W");

  useSeparator();
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
  switch (type) {
    case WStype_TEXT:
      if (payload[0] == 'L') {
        turnONRelay();
      } else if (payload[0] == 'D') {
        turnOFFRelay();
      }
      break;
    default:
      break;
  }
}

void setup() {
  SCT013.current(pinSCT, 7.0199);
  Serial.begin(115200);

  // Declare the GPIO of the relay as output
  pinMode(pinRelay, OUTPUT);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(wifiConnectionDelay);
    Serial.println("Conectando WiFi...");
  }

  Serial.println("Conectando WiFi");
  Serial.print("Endereço IP atribuído: ");
  Serial.println(WiFi.localIP());

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void loop() {
  webSocket.loop();
  delay(websocketDelay);

  if (relayState == HIGH) {
    measure();
  }
}
