#include <TFMPlus.h>  
#include <ArduinoBLE.h>

BLEService service("9180f838-c19f-4b66-8e90-6ed4264d56f1");
BLEStringCharacteristic characteristic("f1199236-fe1c-4141-a40c-2c646cfdf497", BLERead | BLENotify, 100);


TFMPlus leftLidar;
TFMPlus rightLidar;  

#define RXD1 14  
#define TXD1 15

#define RXD2 16 
#define TXD2 17

void setup() {

  Serial.begin(115200);
  delay(20);

  setupBLE();

  Serial1.begin(115200, SERIAL_8N1, RXD1, TXD1);
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);

  delay(20);

  leftLidar.begin(&Serial1);
  rightLidar.begin(&Serial2);

  delay(500);
}

int16_t leftDist = 0;    
int16_t leftFlux = 0;    
int16_t leftTemp = 0;

int16_t rightDist = 0;    
int16_t rightFlux = 0;    
int16_t rightTemp = 0;

void loop() { 

  BLEDevice central = BLE.central();

  if(central){
    Serial.print("Connected to central: ");
    Serial.print(central.address());

    while(central.connected()){

      delay(125);

      String printString = "";

      if(leftLidar.getData(leftDist, leftFlux, leftTemp) && rightLidar.getData(rightDist, rightFlux, rightTemp))
      {
        printString.concat(leftDist);
        printString.concat(":");
        printString.concat(rightDist);

        Serial.println(printString);
        characteristic.writeValue(printString);
      }
    }
    
    Serial.print("Disonnected from central: ");
    Serial.print(central.address());    
  }

  

}

void setupBLE(){
  if(!BLE.begin()){
      Serial.println("starting BLE failed");
      while(1);
    }    

  BLE.setLocalName("Utrasonic sensor");
  BLE.setAdvertisedService(service);
  service.addCharacteristic(characteristic);
  BLE.addService(service);
  BLE.advertise();
  Serial.println("Bluetooth is active waiting for connection...");
}