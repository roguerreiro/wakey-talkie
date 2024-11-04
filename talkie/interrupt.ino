#include <Arduino.h>
#include <FS.h>
#include <SPIFFS.h>

const int dacPin = 25; // Use GPIO 25 for internal DAC output
const int STOP_BTN = 34;
volatile int count = 0;

volatile bool stopFlag = false;

void IRAM_ATTR stopAlarm()
{
  stopFlag = true;
}

void playWAV(String fileName)
{
  // Open the WAV file
  File audioFile = SPIFFS.open(fileName, "r"); // Change to your file name
  if (!audioFile) {
    Serial.println("Failed to open file");
    delay(3000);
    return;
  }
  // Skip WAV file header (first 44 bytes for standard PCM WAV)
  audioFile.seek(44);
  Serial.println(audioFile.available());
  delay(2000);

  // Enable button interrupt
  pinMode(STOP_BTN, INPUT);
  attachInterrupt(STOP_BTN, stopAlarm, RISING);
  
  // Read and play audio data
  while (audioFile.available() && stopFlag==0) {
    // Read a byte from the file
//    Serial.println(stopFlag);
    int sample = audioFile.read();
    // Output to DAC (scale to 0-255)
    if (sample >= 0) {
      dacWrite(dacPin, sample);
    }
    delayMicroseconds(40);
  }

  // Close the file after playback
  audioFile.close();

  detachInterrupt(STOP_BTN);
}

void playAlarm()
{
  while(!stopFlag)
  {
    playWAV("/ceilings16.wav");
  }
  stopFlag = false;
  Serial.println("ok we're done");
}

void setup() {
  Serial.begin(115200);
  
  // Initialize SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS mount failed");
    delay(3000);
    return;
  }

  playAlarm();
}

void loop() {
  // Nothing to do here
  
}
