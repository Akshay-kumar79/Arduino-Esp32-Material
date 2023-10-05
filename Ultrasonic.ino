#include <ArduinoBLE.h>

BLEService service("9180f838-c19f-4b66-8e90-6ed4264d56f1");
BLEStringCharacteristic characteristic("f1199236-fe1c-4141-a40c-2c646cfdf497", BLERead | BLENotify, 100);

const int left_trigPin = 2;
const int left_echoPin = 3;
const int right_trigPin = 7;
const int right_echoPin = 8;

//define sound speed in cm/uS
#define SOUND_SPEED 0.034
#define CM_TO_INCH 0.393701

long duration;
float distanceCm;
float distanceInch;

void setup() {
  
  Serial.begin(9600); // Starts the serial communication

  setupBLE();
  while (!Serial);

  pinMode(left_trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(left_echoPin, INPUT); // Sets the echoPin as an Input
  pinMode(right_trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(right_echoPin, INPUT); // Sets the echoPin as an Input
}

void loop() {

  BLEDevice central = BLE.central();

  if(central){
    Serial.print("Connected to central: ");
    Serial.print(central.address());

    while(central.connected()){

      // Clears the trigPin
      digitalWrite(left_trigPin, LOW);
      delayMicroseconds(2);
      // Sets the trigPin on HIGH state for 10 micro seconds
      digitalWrite(left_trigPin, HIGH);
      delayMicroseconds(10);
      digitalWrite(left_trigPin, LOW);

      // Reads the echoPin, returns the sound wave travel time in microseconds
      duration = pulseIn(left_echoPin, HIGH);

      // Calculate the distance
      distanceCm = duration * SOUND_SPEED / 2;

      // Convert to inches
      distanceInch = distanceCm * CM_TO_INCH;

      // Prints the distance in the Serial Monitor
      String printString = "";
      printString.concat(distanceCm);
      printString.concat(":");
    
      //right us sensor
      digitalWrite(right_trigPin, LOW);
      delayMicroseconds(2);
      // Sets the trigPin on HIGH state for 10 micro seconds
      digitalWrite(right_trigPin, HIGH);
      delayMicroseconds(10);
      digitalWrite(right_trigPin, LOW);

      // Reads the echoPin, returns the sound wave travel time in microseconds
      duration = pulseIn(right_echoPin, HIGH);

      // Calculate the distance
      distanceCm = duration * SOUND_SPEED / 2;

      // Convert to inches
      distanceInch = distanceCm * CM_TO_INCH;

      // Prints the distance in the Serial Monitor
      printString.concat(distanceCm);

      Serial.println(printString);
      characteristic.writeValue(printString);

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

  BLE.setLocalName("Utrasonic sensor");
  BLE.setAdvertisedService(service);
  service.addCharacteristic(characteristic);
  BLE.addService(service);
  BLE.advertise();
  Serial.println("Bluetooth is active waiting for connection...");
}