#include "Sensing.h"

// Use HardwareSerial for ESP32
HardwareSerial radarSerial(1);
SleepBreathingRadar radar;

const uint8_t cuc_CRCHi[256] = {/*...*/}; // Add CRC High Byte Table
const uint8_t cuc_CRCLo[256] = {/*...*/}; // Add CRC Low Byte Table

// CRC16 Calculation
uint16_t calculateCRC16(uint8_t *data, uint16_t length) {
  uint8_t crcHi = 0xFF, crcLo = 0xFF;
  while (length--) {
    uint8_t index = crcLo ^ *data++;
    crcLo = crcHi ^ cuc_CRCHi[index];
    crcHi = cuc_CRCLo[index];
  }
  return (crcHi << 8) | crcLo;
}

// Send Command to Enable Sleep Detection
void sendSleepQuery() {
  uint8_t commandFrame[9] = {
    0x55,       // Start Byte
    0x00, 0x00, // Length (Low, High) - Total 9 bytes
    0x01,       // Function Code: Write
    0x05,       // Address1: Sleep Detection
    0x0D,       // Address2: Sleep Switch
    0x00,       // Data: Enable (0x01)
    0x00, 0x00  // Placeholder for CRC
  };

  // Calculate CRC
  uint16_t crc = calculateCRC16(commandFrame, 7); // Exclude CRC bytes
  commandFrame[7] = crc & 0xFF; // CRC Low byte
  commandFrame[8] = (crc >> 8) & 0xFF; // CRC High byte

  // Send Command
  radarSerial.write(commandFrame, sizeof(commandFrame));
  // Serial.println("Sleep detection switch command sent.");
}

// Function to process radar frame
void processRadarFrame(const uint8_t radarDataBuffer[]) {
  if (radarDataBuffer[0] != START_BYTE) {
    Serial.println("Invalid frame start. Discarding frame.");
    return;
  }
  Serial.print("Raw Frame: ");
  for (size_t i = 0; i < sizeof(radarDataBuffer)*8; i++) {
    if (radarDataBuffer[i] < 0x10) Serial.print("0");
    Serial.print(radarDataBuffer[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
}

bool isPersonActive(){

    int act_count = 0;
    int aval_count = 0;

    for (int i = 0; i++; i<SENSE_TIMES){
        if(digitalRead(ACTIVITY_PIN)){
            act_count ++;
        }

        if (digitalRead(AVAIL_PIN)){
            aval_count++;
        }
    }

    if ((act_count>SENSE_THRESHOLD) && (aval_count>SENSE_THRESHOLD)){
        return true;
    }

    return false;
}

void getData() {
  radar.recvRadarBytes(); // Receive radar data and start processing
  if (radar.newData == true) { // The data is received and transferred to the new list dataMsg[]
    byte dataMsg[radar.dataLen + 1] = {0x00};
    dataMsg[0] = 0x55; // Add the header frame as the first element of the array
    for (byte n = 0; n < radar.dataLen; n++) dataMsg[n + 1] = radar.Msg[n]; // Frame-by-frame transfer
    radar.newData = false; // A complete set of data frames is saved

    // radar.ShowData(dataMsg); // Serial port prints a set of received data frames
    radar.Sleep_inf(dataMsg); // Sleep information output
    processRadarFrame(dataMsg); // Process the received frame
    Serial.println(radar.CURRENT_STATE);
  }
}

void senseSetup() {
  radarSerial.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);
  radar.SerialInit(&radarSerial); // Pass the hardware serial instance to the radar object
  pinMode(AVAIL_PIN, INPUT); // Initialize GPIO pins
  pinMode(ACTIVITY_PIN, INPUT);
  delay(500);
}

