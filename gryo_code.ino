#include <SPI.h>

// Pin definitions
#define CS_PIN 10 // Chip Select for ICM-20948

// Timer variables
unsigned long startTime;
const unsigned long oneMinute = 60000; // 1 minute in milliseconds

// ICM-20948 register addresses
#define WHO_AM_I 0x00
#define PWR_MGMT_1 0x07
#define ACCEL_XOUT_H 0x2D

// SPI settings (adjust as per your ICM-20948 datasheet)
SPISettings settings(1000000, MSBFIRST, SPI_MODE3); // 1MHz, MSB first, SPI mode 3

void setup() {
  // Initialize Serial Monitor
  Serial.begin(9600);
  while (!Serial); // Wait for the Serial Monitor to open

  // Initialize SPI
  SPI.begin();
  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH); // Deselect the sensor

  // Verify communication with ICM-20948
  uint8_t whoAmI = readICM20948(WHO_AM_I);
  if (whoAmI == 0xEA) { // ICM-20948 WHO_AM_I register should return 0xEA
    Serial.println("ICM-20948 detected!");
  } else {
    Serial.print("ICM-20948 not found. WHO_AM_I = 0x");
    Serial.println(whoAmI, HEX);
  }

  // Configure ICM-20948 (e.g., wake up from sleep)
  writeICM20948(PWR_MGMT_1, 0x00); // Set PWR_MGMT_1 to 0x01 to exit sleep mode

  // Start the one-minute timer
  startTime = millis();
}

void loop() {
  // Check if one minute has passed
  if (millis() - startTime < oneMinute) {
    // Read accelerometer and gyroscope data every second
    static unsigned long lastReadTime = 0;
    if (millis() - lastReadTime >= 1000) {
      lastReadTime = millis();
      readSensorData();
    }
  } else {
    Serial.println("1-minute data capture complete.");
    while (true); // Stop execution after 1 minute
  }
}

void readSensorData() {
  // Read accelerometer data (X, Y, Z)
  int16_t accelX = (readICM20948(ACCEL_XOUT_H) << 8) | readICM20948(ACCEL_XOUT_H + 1);
  int16_t accelY = (readICM20948(ACCEL_XOUT_H + 2) << 8) | readICM20948(ACCEL_XOUT_H + 3);
  int16_t accelZ = (readICM20948(ACCEL_XOUT_H + 4) << 8) | readICM20948(ACCEL_XOUT_H + 5);
  
  Serial.print("Accel X: ");
  Serial.print(accelX);
  Serial.print(", Y: ");
  Serial.print(accelY);
  Serial.print(", Z: ");
  Serial.println(accelZ);
}

void writeICM20948(uint8_t reg, uint8_t data) {
  digitalWrite(CS_PIN, LOW); // Select the sensor
  SPI.beginTransaction(settings);
  SPI.transfer(reg & 0x7F); // Write operation (MSB=0)
  SPI.transfer(data);
  SPI.endTransaction();
  digitalWrite(CS_PIN, HIGH); // Deselect the sensor
}

uint8_t readICM20948(uint8_t reg) {
  digitalWrite(CS_PIN, LOW); // Select the sensor
  SPI.beginTransaction(settings);
  SPI.transfer(reg | 0x80); // Read operation (MSB=1)
  uint8_t data = SPI.transfer(0x00); // Send dummy byte to receive data
  SPI.endTransaction();
  digitalWrite(CS_PIN, HIGH); // Deselect the sensor
  return data;
}
