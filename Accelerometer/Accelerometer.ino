#include "Wire.h"
#include "LSM6DS3.h"
#include <ArduinoBLE.h>

BLEService service("e7ba2b08-cd66-4c00-8dff-97b1046afb7c");
BLEStringCharacteristic characteristic("c9359722-ccb2-4170-a5ee-dc64e96fd823", BLERead | BLENotify, 100);

//Create a instance of class LSM6DS3
LSM6DS3 myIMU(I2C_MODE, 0x6A);    //I2C device address 0x6A

float accX;

void setup() {
    // put your setup code here, to run once:
    Serial.begin(9600);
    
    setupBLE();
    while (!Serial);
    //Call .begin() to configure the IMUs
    myIMU.begin();
}

void loop() {

  BLEDevice central = BLE.central();

  if(central){
    Serial.print("Connected to central: ");
    Serial.print(central.address());

    while(central.connected()){

      //Accelerometer
      
      String accelerometerStr = "";
      accelerometerStr.concat(myIMU.readFloatAccelX() * 100);
      
      Serial.println(accelerometerStr);
      characteristic.writeValue(accelerometerStr);    

      delay(125);
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

  BLE.setLocalName("Accelerometer");
  BLE.setAdvertisedService(service);
  service.addCharacteristic(characteristic);
  BLE.addService(service);
  BLE.advertise();
  Serial.println("Bluetooth is active waiting for connection...");
}



  