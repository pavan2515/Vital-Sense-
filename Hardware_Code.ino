#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"
#include "spo2_algorithm.h"
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#define SDA_PIN 4
#define SCL_PIN 5

// WiFi settings - must match sender
const char* ssid = "Redmi";
const char* password = "12345678";
const int udpPort = 1234;

MAX30105 particleSensor;
WiFiUDP udp;

// Heart rate variables
const byte RATE_SIZE = 4;
byte rates[RATE_SIZE];
byte rateSpot = 0;
long lastBeat = 0;
float beatsPerMinute;
int beatAvg;

// SpO2 variables
#define MAX_BRIGHTNESS 255
uint32_t irBuffer[100];
uint32_t redBuffer[100];
int32_t spo2;
int8_t validSPO2;
int32_t heartRate;
int8_t validHeartRate;

// MPU variables (received from sender)
float mpu1AccX, mpu1AccY, mpu1AccZ;
float mpu2AccX, mpu2AccY, mpu2AccZ;
unsigned long lastMpuUpdate = 0;

// Timing variables
unsigned long lastSpo2Calculation = 0;
const unsigned long spo2CalculationInterval = 4000;
unsigned long lastDataPrint = 0;
const unsigned long printInterval = 200;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected! IP: " + WiFi.localIP().toString());
  udp.begin(udpPort);

  // Initialize MAX30102
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(400000);

  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("MAX30102 not found");
    while (1);
  }

  // Configure sensor
  particleSensor.setup(60, 1, 2, 400, 411, 4096);
  particleSensor.enableDIETEMPRDY();

  Serial.println("Place finger on sensor for readings");
}

void loop() {
  // Check for incoming MPU data
  int packetSize = udp.parsePacket();
  if (packetSize) {
    char incomingPacket[50];
    int len = udp.read(incomingPacket, 50);
    if (len > 0) {
      incomingPacket[len] = 0;
      processMpuData(incomingPacket);
      lastMpuUpdate = millis();
    }
  }

  // Original MAX30102 code (unchanged)
  long irValue = particleSensor.getIR();
  
  if (checkForBeat(irValue)) {
    long delta = millis() - lastBeat;
    lastBeat = millis();
    beatsPerMinute = 60 / (delta / 1000.0);

    if (beatsPerMinute < 255 && beatsPerMinute > 20) {
      rates[rateSpot++] = (byte)beatsPerMinute;
      rateSpot %= RATE_SIZE;
      beatAvg = 0;
      for (byte x = 0; x < RATE_SIZE; x++) beatAvg += rates[x];
      beatAvg /= RATE_SIZE;
    }
  }

  static int spo2BufferIndex = 0;
  if (particleSensor.available()) {
    redBuffer[spo2BufferIndex] = particleSensor.getRed();
    irBuffer[spo2BufferIndex] = particleSensor.getIR();
    particleSensor.nextSample();
    
    if (++spo2BufferIndex >= 100) {
      spo2BufferIndex = 0;
      if (millis() - lastSpo2Calculation >= spo2CalculationInterval) {
        maxim_heart_rate_and_oxygen_saturation(irBuffer, 100, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
        lastSpo2Calculation = millis();
      }
    }
  }

  // Combined data output
  if (millis() - lastDataPrint >= printInterval) {
    static unsigned int printCount = 0;
    if (printCount++ % 25 == 0) {
      Serial.println("\nTime\tIR\tBPM\tAvgBPM\tSpO2%\tMPU1 Acc\t\tMPU2 Acc");
    }

  
    
    if (irValue > 50000) {
      Serial.print(beatsPerMinute);
      Serial.print("\t");
      Serial.print(beatAvg);
      Serial.print("\t");
      Serial.print(validSPO2 ? spo2 : 0);
    } else {
      Serial.print("---\t---\t---");
    }

    Serial.print("\t");
    // Print MPU data if received within last 2 seconds
    if (millis() - lastMpuUpdate < 2000) {
      Serial.print(mpu1AccX, 3); Serial.print(",");  // Changed to 6 decimal places
      Serial.print(mpu2AccX, 3); Serial.print(",");
      Serial.print(mpu1AccY, 3); Serial.print("\t");
      Serial.print(mpu2AccY, 3); Serial.print(",");
      Serial.print(mpu1AccZ, 3); Serial.print(",");
      Serial.print(mpu2AccZ, 3);
    } else {
      Serial.print("---,---,---\t---,---,---");
    }
    
    Serial.println();
    lastDataPrint = millis();
  }
}

void processMpuData(char* data) {
  char* token = strtok(data, ",");
  if (token) mpu1AccX = atof(token);
  token = strtok(NULL, ",");
  if (token) mpu1AccY = atof(token);
  token = strtok(NULL, ",");
  if (token) mpu1AccZ = atof(token);
  token = strtok(NULL, ",");
  if (token) mpu2AccX = atof(token);
  token = strtok(NULL, ",");
  if (token) mpu2AccY = atof(token);
  token = strtok(NULL, ",");
  if (token) mpu2AccZ = atof(token);
}