#include "BluetoothSerial.h"

BluetoothSerial bt;
int a = 0;

void setup() {
  Serial.begin(9600);
  bt.begin("Esp32 classic");
}

void loop() {
  a++;
  Serial.println(a);
  bt.print(a);
  delay(500);
}
