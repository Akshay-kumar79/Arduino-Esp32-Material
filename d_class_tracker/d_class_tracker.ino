#include <SoftwareSerial.h>
#include <Wire.h>

// Pins
#define RIGHT_ECHO_PIN 2
#define RIGHT_TRIG_PIN 3
#define LEFT_ECHO_PIN 4
#define LEFT_TRIG_PIN 5

// MPU registers
#define MPU6050_ADDR                  0x68
#define MPU6050_SMPLRT_DIV_REGISTER   0x19
#define MPU6050_CONFIG_REGISTER       0x1a
#define MPU6050_PWR_MGMT_1_REGISTER   0x6b
#define MPU6050_GYRO_Z_OUT_REGISTER   0x47

// MPU6050 variables
float gyroZoffset = 0.72;
float gyroZ;
float angleZ;
float preInterval;

// Bluetooth
SoftwareSerial mySerial(10, 9); //( RX -> 9, TX -> 10) Connect TX of bluetooth to pin 10 and RX of bluetooth to pin 9 

// Ultrasonic sensor
int right_distance;
int left_distance;


enum USDirection{
  ULTRA_SONIC_RIGHT,
  ULTRA_SONIC_LEFT
};

void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);

  // MPU6050
  Wire.begin();
  connectMPU();

  // Ultrasonic sensor
  pinMode(RIGHT_TRIG_PIN, OUTPUT);
  pinMode(RIGHT_ECHO_PIN, INPUT);
  pinMode(LEFT_TRIG_PIN, OUTPUT);
  pinMode(LEFT_ECHO_PIN, INPUT);
}

void loop() {

  String outputString = "";

  updateMPUData();
  outputString.concat(angleZ);
  outputString.concat(":");

//  updateDistance(USDirection::ULTRA_SONIC_RIGHT);
//  outputString.concat(right_distance);
//  outputString.concat(":");
//
//  updateDistance(USDirection::ULTRA_SONIC_LEFT);
//  outputString.concat(left_distance);

  mySerial.println(outputString);
  Serial.println(outputString);

  delay(125);

}

// right side is default
void updateDistance(USDirection dir){

  switch (dir) {
    case USDirection::ULTRA_SONIC_LEFT: {

      digitalWrite(LEFT_TRIG_PIN, LOW);
      delayMicroseconds(2);
      digitalWrite(LEFT_TRIG_PIN, HIGH);
      delayMicroseconds(10);
      digitalWrite(LEFT_TRIG_PIN, LOW);

      long duration = pulseIn(LEFT_ECHO_PIN, HIGH);
      left_distance = (duration * 0.034 / 2);
      break;

    }

    case USDirection::ULTRA_SONIC_RIGHT: {

      digitalWrite(RIGHT_TRIG_PIN, LOW);
      delayMicroseconds(2);
      digitalWrite(RIGHT_TRIG_PIN, HIGH);
      delayMicroseconds(10);
      digitalWrite(RIGHT_TRIG_PIN, LOW);

      long duration = pulseIn(RIGHT_ECHO_PIN, HIGH);
      right_distance = (duration * 0.034 / 2);
      break;

    }

    default: {

      digitalWrite(RIGHT_TRIG_PIN, LOW);
      delayMicroseconds(2);
      digitalWrite(RIGHT_TRIG_PIN, HIGH);
      delayMicroseconds(10);
      digitalWrite(RIGHT_TRIG_PIN, LOW);

      long duration = pulseIn(RIGHT_ECHO_PIN, HIGH);
      right_distance = (duration * 0.034 / 2);
      break;

    }
  }

}


void connectMPU(){
  writeDataInMPU(MPU6050_PWR_MGMT_1_REGISTER, 0x01); // check only the first connection with status
  writeDataInMPU(MPU6050_SMPLRT_DIV_REGISTER, 0x00);
  writeDataInMPU(MPU6050_CONFIG_REGISTER, 0x00);
}

byte writeDataInMPU(byte reg, byte data){
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(reg);
  Wire.write(data);
  byte status = Wire.endTransmission();
  return status; // 0 if success
}

void updateMPUData(){
  // retrieve raw data
  fetchGyroZRawData();
  
  float currentMilli = millis();
  float dt = (currentMilli - preInterval) / 1000; // devided by 1000 to get second from millisecond
  preInterval = currentMilli;

  angleZ += gyroZ*dt; // processed gyro angle in degree
}

void fetchGyroZRawData(){
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(MPU6050_GYRO_Z_OUT_REGISTER);
  Wire.endTransmission(false);
  Wire.requestFrom((uint8_t)MPU6050_ADDR ,(uint8_t) 2);

	int16_t rawGyroZData = Wire.read() << 8 | Wire.read();
  gyroZ = ((float)rawGyroZData) / 131 - gyroZoffset;
}
