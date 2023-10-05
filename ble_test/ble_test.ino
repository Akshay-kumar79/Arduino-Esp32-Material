#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
BLECharacteristic *pCharacteristic;

int txValue = 0;

void setup() {
  Serial.begin(9600);
  setupBLE();  
}

void loop() {

  txValue++;

  String printString = "";
  printString.concat(txValue);
  printString.concat(": is printed value second");
  printString.concat(" line");

  delay(10);

  printBLE(printString);
  Serial.println(printString);
  delay(1000);
    
}

void setupBLE(){
  BLEDevice::init("MyESP32");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);

  pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_NOTIFY |
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  pCharacteristic->addDescriptor(new BLE2902());
  pService->start();

  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
}

void printBLE(String output){
  int n = output.length();
  char stringOutput[n + 1];
  strcpy(stringOutput, output.c_str()); 

  pCharacteristic->setValue(stringOutput);
  pCharacteristic->notify();
}