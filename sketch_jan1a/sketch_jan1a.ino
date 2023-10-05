#include <LSM6DS3.h> 
#include <Wire.h>

#define PIN 10
#define NUMPIXELS 64

LSM6DS3 lsm6ds33(I2C_MODE, 0x6A); //I2C device address 0x6A

float temperature, pressure, altitude;
float magnetic_x, magnetic_y, magnetic_z;
float humidity;
float accelX, accelY, accelZ, // units m/s/s i.e. accelZ if often 9.8 (gravity)
gyroX, gyroY, gyroZ, // units dps (degrees per second)
gyroDriftX, gyroDriftY, gyroDriftZ, // units dps
gyroRoll, gyroPitch, gyroYaw, // units degrees (expect major drift)
gyroCorrectedRoll, gyroCorrectedPitch, gyroCorrectedYaw, // units degrees (expect minor drift)
accRoll, accPitch, accYaw, // units degrees (roll and pitch noisy, yaw not possible)
complementaryRoll, complementaryPitch, complementaryYaw; // units degrees (excellent roll, pitch, yaw minor drift)
uint8_t roll, pitch, yaw;
long lastTime;
long lastInterval;

bool readIMU() {
  accelX = lsm6ds33.readFloatAccelX();
  accelY = lsm6ds33.readFloatAccelY();
  accelZ = lsm6ds33.readFloatAccelZ();
  gyroX = lsm6ds33.readFloatGyroX();
  gyroY = lsm6ds33.readFloatGyroY();
  gyroZ = lsm6ds33.readFloatGyroZ();
  return true;
}

/*
the gyro's x,y,z values drift by a steady amount. if we measure this when arduino is still
we can correct the drift when doing real measurements later
*/
void calibrateIMU(int delayMillis, int calibrationMillis) {
  int calibrationCount = 0;
  delay(delayMillis); // to avoid shakes after pressing reset button
  float sumX, sumY, sumZ;
  int startTime = millis();
  while (millis() < startTime + calibrationMillis) {
    if (readIMU()) {
      // in an ideal world gyroX/Y/Z == 0, anything higher or lower represents drift
      sumX += gyroX;
      sumY += gyroY;
      sumZ += gyroZ;
      calibrationCount++;
    }
  }
  if (calibrationCount == 0) {
    Serial.println("Failed to calibrate");
  }
  gyroDriftX = sumX / calibrationCount;
  gyroDriftY = sumY / calibrationCount;
  gyroDriftZ = sumZ / calibrationCount;

  Serial.println("Calibration success");
}

void doImuCalculations() {
  accRoll = atan2(accelY, accelZ) * 180 / M_PI;
  accPitch = atan2(-accelX, sqrt(accelY * accelY + accelZ * accelZ)) * 180 / M_PI;
  float lastFrequency = (float) 1000000.0 / lastInterval;
  gyroRoll = gyroRoll + (gyroX / lastFrequency);
  gyroPitch = gyroPitch + (gyroY / lastFrequency);
  gyroYaw = gyroYaw + (gyroZ / lastFrequency);
  gyroCorrectedRoll = gyroCorrectedRoll + ((gyroX - gyroDriftX) / lastFrequency);
  gyroCorrectedPitch = gyroCorrectedPitch + ((gyroY - gyroDriftY) / lastFrequency);
  gyroCorrectedYaw = gyroCorrectedYaw + ((gyroZ - gyroDriftZ) / lastFrequency);
  complementaryRoll = complementaryRoll + ((gyroX - gyroDriftX) / lastFrequency);
  complementaryPitch = complementaryPitch + ((gyroY - gyroDriftY) / lastFrequency);
  complementaryYaw = complementaryYaw + ((gyroZ - gyroDriftZ) / lastFrequency);
  complementaryRoll = 0.98 * complementaryRoll + 0.02 * accRoll;
  complementaryPitch = 0.98 * complementaryPitch + 0.02 * accPitch;
  roll = map(complementaryRoll, -180, 180, 0, 255);
  pitch = map(complementaryPitch, -180, 180, 0, 255);
  yaw = map(complementaryYaw, -180, 180, 0, 255);
}
/**
This comma separated format is best 'viewed' using 'serial plotter' or processing.org client (see https://github.com/arduino-libraries/Arduino_LSM6DS3/blob/5eac7f5e6145c4747da27698faf3a548d2893a2b/examples/RollPitchYaw/data/processing/RollPitchYaw3d.pde)
*/
void printCalculations() {
// Serial.print(gyroRoll);
// Serial.print(',');
// Serial.print(gyroPitch);
// Serial.print(',');
// Serial.print(gyroYaw);
// Serial.print(',');
// Serial.print(gyroCorrectedRoll);
// Serial.print(',');
// Serial.print(gyroCorrectedPitch);
// Serial.print(',');
// Serial.print(gyroCorrectedYaw);
// Serial.print(',');
// Serial.print(accRoll);
// Serial.print(',');
// Serial.print(accPitch);
// Serial.print(',');
// Serial.print(accYaw);
// Serial.print(',');
// Serial.print(complementaryRoll);
// Serial.print(',');
// Serial.print(complementaryPitch);
// Serial.print(',');
 Serial.print(complementaryYaw);

//Serial.print(gyroX);
//Serial.print(',');
//Serial.print(gyroY);
//Serial.print(',');
//Serial.print(gyroZ);

//Serial.println("");
// Serial.print(roll/255.0);
// Serial.print(',');
// Serial.print(pitch/255.0);
// Serial.print(',');
// Serial.print(yaw);
 Serial.println("");
delay(500);
}

void setup() {
  Serial.begin(9600);

  if (lsm6ds33.begin() != 0) {
    Serial.println("Device error");
  } else {
    Serial.println("Device OK!");
  }

  calibrateIMU(1000, 5000);
}

void loop() {
  if (readIMU()) {
    long currentTime = micros();
    lastInterval = currentTime - lastTime;
    lastTime = currentTime;
    doImuCalculations();
    printCalculations();
  }
}
