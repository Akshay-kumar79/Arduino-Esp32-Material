#include <ArduinoBLE.h>
#include <SoftwareSerial.h>   

SoftwareSerial SerialLeft; 
SoftwareSerial SerialRight; 

BLEService service("9180f838-c19f-4b66-8e90-6ed4264d56f1");
BLEStringCharacteristic characteristic("f1199236-fe1c-4141-a40c-2c646cfdf497", BLERead | BLENotify, 100);

int leftDist, rightDist;
int check;                    //save check value
int i;
int uart[9];                   //save data measured by LiDAR
const int HEADER = 0x59; 

void setup() {
  
  Serial.begin(9600); // Starts the serial communication

  setupBLE();
  while (!Serial);

  SerialLeft.begin(38400, SWSERIAL_8N1, 2, 3, false);
  SerialRight.begin(38400, SWSERIAL_8N1, 7, 8, false);

  while(!SerialLeft || !SerialRight){
    Serial.println("Invalid SoftwareSerial pin configuration, check config"); 
    delay (1000);
  }

}

void loop() {

  BLEDevice central = BLE.central();

  if(central){
    Serial.print("Connected to central: ");
    Serial.print(central.address());

    while(central.connected()){

      if (SerialLeft.available() && SerialRight.available())                //check if serial port has data input
      {
        String printString = "";

        if (SerialLeft.read() == HEADER)        //assess data package frame header 0x59
        {
          uart[0] = HEADER;
          if (SerialLeft.read() == HEADER)      //assess data package frame header 0x59
          {
            uart[1] = HEADER;
            for (i = 2; i < 9; i++)         //save data in array
            {
              uart[i] = SerialLeft.read();
            }
            check = uart[0] + uart[1] + uart[2] + uart[3] + uart[4] + uart[5] + uart[6] + uart[7];
            if (uart[8] == (check & 0xff))        //verify the received data as per protocol
            {
              leftDist = uart[2] + uart[3] * 256;     //calculate distance value
              
              printString.concat(leftDist);
              printString.concat(":");
            }
          }
        }


        if (SerialRight.read() == HEADER)        //assess data package frame header 0x59
        {
          uart[0] = HEADER;
          if (SerialRight.read() == HEADER)      //assess data package frame header 0x59
          {
            uart[1] = HEADER;
            for (i = 2; i < 9; i++)         //save data in array
            {
              uart[i] = SerialRight.read();
            }
            check = uart[0] + uart[1] + uart[2] + uart[3] + uart[4] + uart[5] + uart[6] + uart[7];
            if (uart[8] == (check & 0xff))        //verify the received data as per protocol
            {
              rightDist = uart[2] + uart[3] * 256;     //calculate distance value
              
              printString.concat(rightDist);
            }
          }
        }

        Serial.println(printString);
        characteristic.writeValue(printString);

      }
    }
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