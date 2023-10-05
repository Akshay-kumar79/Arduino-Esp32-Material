#include <WebSocketsServer.h>
#include <WiFi.h>
#include "camera_wrap.h"

const char* ssid = "ESP32 CAM";    // <<< change this as yours
const char* password = "esp12345"; // <<< change this as yours

uint8_t clientNo=0;
bool clientConnected = false;

uint8_t* jpgBuff = new uint8_t[68123];
size_t   jpgLength = 0;

WebSocketsServer webSocket = WebSocketsServer(86);

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

  switch(type) {
      case WStype_DISCONNECTED:
          Serial.printf("[%u] Disconnected!\n", num);
          clientNo = num;
          clientConnected = false;
          break;
      case WStype_CONNECTED:
          Serial.printf("[%u] Connected!\n", num);
          clientConnected = true;
          break;
      case WStype_TEXT:
      case WStype_BIN:
      case WStype_ERROR:
      case WStype_FRAGMENT_TEXT_START:
      case WStype_FRAGMENT_BIN_START:
      case WStype_FRAGMENT:
      case WStype_FRAGMENT_FIN:
          Serial.println(type);
          break;
  }
}

void setup() {  
  Serial.begin(115200);

  int cameraInitState = initCamera();

  Serial.printf("camera init state %d\n", cameraInitState);

  if(cameraInitState != 0){
    return;
  }

  WiFi.softAP(ssid, password);

  Serial.println("");
  Serial.print("IP address: ");
  String ipAddress = WiFi.softAPIP().toString();
  // String ipAddress = WiFi.localIP().toString();
  Serial.println(ipAddress);

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

}

void loop() {
   webSocket.loop();
  if(clientConnected == true){
    grabImage(jpgLength, jpgBuff);
    webSocket.sendBIN(clientNo, jpgBuff, jpgLength);
  }
}
