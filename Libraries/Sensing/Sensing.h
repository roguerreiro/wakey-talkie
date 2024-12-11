#ifndef SENSING_H
#define SENSING_H

#include <sleepbreathingradar.h>
#include <Arduino.h>
#include <HardwareSerial.h>

// Define RX and TX pins for ESP32 (adjust based on your wiring)
#define RX_PIN 16 // ESP32 RX pin connected to MR24BSD1 TX
#define TX_PIN 17 // ESP32 TX pin connected to MR24BSD1 RX
#define AVAIL_PIN 34 // GPIO pin for availability status
#define ACTIVITY_PIN 35// GPIO pin for activity status
#define START_BYTE 0x55 // Start byte for valid frames

// Forward declarations (no definitions)
extern HardwareSerial radarSerial;  // Declare radarSerial object
extern SleepBreathingRadar radar;   // Declare radar object

extern const uint8_t cuc_CRCHi[256]; // Declare CRC High Byte Table
extern const uint8_t cuc_CRCLo[256]; // Declare CRC Low Byte Table

// Function declarations
void senseSetup();
bool isPersonActive();

#define FRAME_SIZE 20
#define SENSE_TIMES 5
#define SENSE_THRESHOLD 2

#endif // SENSING_H
