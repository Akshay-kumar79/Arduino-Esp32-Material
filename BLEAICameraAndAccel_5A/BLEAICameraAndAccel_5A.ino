#include <bluefruit.h>
#include <Adafruit_LittleFS.h>
#include <InternalFileSystem.h>
#include "Seeed_Arduino_GroveAI.h"
#include <Wire.h>  // Arduino standard I2C/Two-Wire Library
#include "LSM6DS3.h"
#include "Wire.h"

GroveAI ai(Wire);
uint8_t state = 0;
int x, y, w, h, confidence;

float accelx, accely, accelz;

// BLE Service
BLEDfu bledfu;    // OTA DFU service
BLEDis bledis;    // device information
BLEUart bleuart;  // uart over ble
BLEBas blebas;    // battery

LSM6DS3 myIMU(I2C_MODE, 0x6A);  //I2C device address 0x6A

void setup() {
  Serial.begin(115200);  // Initialize terminal serial port
  delay(20);

  setupBLE();
  setupAI();
  if (myIMU.begin() != 0) {
    Serial.println("Accelerometer ERROR");
  } else {
    Serial.println("Accelerometer OK!");
  }
  //ai.imu_start();
}

void setupAI() {
  if (ai.begin(ALGO_OBJECT_DETECTION, MODEL_EXT_INDEX_1))  // Object detection and externel model 1
  {
    Serial.print("Version: 0x");
    Serial.println(ai.version(), HEX);
    Serial.print("ID: 0x");
    Serial.println(ai.id(), HEX);
    Serial.print("Algo: ");
    Serial.println(ai.algo());
    Serial.print("Model: ");
    Serial.println(ai.model());
    Serial.print("Confidence: ");
    Serial.println(ai.confidence());
    state = 1;
  } else {
    Serial.println("Algo begin failed.");
  }
}

// = = = = = = = = = =  MAIN LOOP  = = = = = = = = = =
void loop() {
  pollAI();
  handleBLE();
  pollAccel();
}
// = = = = = = = = =  End of Main Loop  = = = = = = = = =

void pollAccel() {

  accelx = myIMU.readFloatAccelX();
  accely = myIMU.readFloatAccelY();
  accelz = myIMU.readFloatAccelZ();

  // Serial.print(accelx);
  // Serial.print(", ");
  // Serial.print(accely);
  // Serial.print(", ");
  // Serial.print(accelz);
  // Serial.print(", ");
  // Serial.println();
}

void handleBLE() {

  pollAI();

  String printString = "";
  printString.concat(int(accelx*100));
  printString.concat(":");
  printString.concat(int(accely*100));
  printString.concat(":");
  printString.concat(int(accelz*100));
  printString.concat(":");
  printString.concat(x);
  printString.concat(":");
  printString.concat(y);
  printString.concat(":");
  printString.concat(w);
  printString.concat(":");
  printString.concat(h);
  printString.concat(":");
  printString.concat(confidence);

  byte sendArry[] = {
    highByte(int(accelx*100)), lowByte(int(accelx*100)), 
    highByte(int(accely*100)), lowByte(int(accely*100)), 
    highByte(int(accelz*100)), lowByte(int(accelz*100)),
    highByte(x), lowByte(x),
    highByte(y), lowByte(y),
    highByte(w), lowByte(w),
    highByte(h), lowByte(h),    
    highByte(confidence), lowByte(confidence)
  };

  Serial.println(printString);
  //int count = sizeof(printString);
  bleuart.write(sendArry, 16);
}

void setupBLE() {
  Bluefruit.autoConnLed(true);

  // Config the peripheral connection with maximum bandwidth
  // more SRAM required by SoftDevice
  // Note: All config***() function must be called before begin()
  Bluefruit.configPrphBandwidth(BANDWIDTH_MAX);

  Bluefruit.begin();
  Bluefruit.setTxPower(4);  // Check bluefruit.h for supported values
  //Bluefruit.setName(getMcuUniqueID()); // useful testing with multiple central connections
  Bluefruit.Periph.setConnectCallback(connect_callback);
  Bluefruit.Periph.setDisconnectCallback(disconnect_callback);

  // To be consistent OTA DFU should be added first if it exists
  //bledfu.begin();

  // Configure and Start Device Information Service
  bledis.setManufacturer("TEST");
  bledis.setModel("BLE AI CAMERA");
  bledis.begin();

  // Configure and Start BLE Uart Service
  bleuart.begin();

  // Start BLE Battery Service
  blebas.begin();
  blebas.write(100);

  // Set up and start advertising
  startAdv();
  Serial.println("Bluetooth is active waiting for connection...");
}

void startAdv(void) {
  // Advertising packet
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();

  // Include bleuart 128-bit uuid
  Bluefruit.Advertising.addService(bleuart);

  // Secondary Scan Response packet (optional)
  // Since there is no room for 'Name' in Advertising packet
  Bluefruit.ScanResponse.addName();

  /* Start Advertising
   * - Enable auto advertising if disconnected
   * - Interval:  fast mode = 20 ms, slow mode = 152.5 ms
   * - Timeout for fast mode is 30 seconds
   * - Start(timeout) with timeout = 0 will advertise forever (until connected)
   * 
   * For recommended advertising interval
   * https://developer.apple.com/library/content/qa/qa1931/_index.html   
   */
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);  // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);    // number of seconds in fast mode
  Bluefruit.Advertising.start(0);              // 0 = Don't stop advertising after n seconds
}

void pollAI() {
  if (state == 1) {
    uint32_t tick = millis();
    if (ai.invoke())  // begin invoke
    {
      while (1)  // wait for invoking finished
      {
        CMD_STATE_T ret = ai.state();
        if (ret == CMD_STATE_IDLE) {
          break;
        }
        delay(20);
      }

      uint8_t len = ai.get_result_len();  // receive how many people detect
      if (len) {
        int time1 = millis() - tick;
        //Serial.print("Time consuming: ");
        //Serial.println(time1);

        object_detection_t data;  //get data

        for (int i = 0; i < len; i++) {
          //if (data.confidence > 00) {
          //Serial.print("Number of objects: ");
          //Serial.println(len);
          //Serial.println("result:detected");
          //Serial.print("Detecting and calculating: ");
          //Serial.println(i + 1);
          ai.get_result(i, (uint8_t*)&data, sizeof(object_detection_t));  //get result

          confidence = data.confidence;
          x = data.x;
          y = data.y;
          w = data.w;
          h = data.h;

          // Serial.print("confidence:");
          // Serial.print(confidence);
          // Serial.print(", location: (");
          // Serial.print(x);
          // Serial.print(", ");
          // Serial.print(y);
          // Serial.print(")");
          // Serial.print(", size: (");
          // Serial.print(w);
          // Serial.print(", ");
          // Serial.print(h);
          // Serial.print(")");
          // Serial.println();
          //}
        }
      } else {
        // Serial.println("No identification");
        confidence = 0;
      }
    } else {
      delay(1000);
      Serial.println("Invoke Failed.");
    }
  } else {
    state == 0;
  }
}


// callback invoked when central connects
void connect_callback(uint16_t conn_handle) {
  // Get the reference to current connection
  BLEConnection* connection = Bluefruit.Connection(conn_handle);

  char central_name[32] = { 0 };
  connection->getPeerName(central_name, sizeof(central_name));

  Serial.print("Connected to ");
  Serial.println(central_name);
}

/**
 * Callback invoked when a connection is dropped
 * @param conn_handle connection where this event happens
 * @param reason is a BLE_HCI_STATUS_CODE which can be found in ble_hci.h
 */
void disconnect_callback(uint16_t conn_handle, uint8_t reason) {
  (void)conn_handle;
  (void)reason;

  Serial.println();
  Serial.print("Disconnected, reason = 0x");
  Serial.println(reason, HEX);
}
