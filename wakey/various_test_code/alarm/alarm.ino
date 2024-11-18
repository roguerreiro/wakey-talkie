#include <Arduino.h>
#include <FS.h>
#include <SPIFFS.h>

const int DAC_OUT = 25;
const int STOP_BTN = 32;

volatile bool sampleFlag = false;
volatile bool stopFlag = false;

// for now, assume only one alarm
int repeatCount = 0;

hw_timer_t *sampleTimer = NULL;
File audioFile; 

void IRAM_ATTR isr_stop_alarm()
{
  stopFlag = true;
}

void IRAM_ATTR isr_play_sample()
{
  sampleFlag = true;
}

void triggerAlarm(String alarmFile, int repeats);
void playWAV(String fileName);
void stopAlarm();
void repeatAlarm();

void setup() {
  Serial.begin(115200);

  // Enable button interrupt
  pinMode(STOP_BTN, INPUT);
  attachInterrupt(STOP_BTN, isr_stop_alarm, RISING);
  
  // Initialize SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS mount failed");
    delay(3000);
    return;
  }
  triggerAlarm("/ceilings16.wav", 3);
}

void loop() 
{
  if(sampleFlag) // time to play a sample
  {
    sampleFlag = false; 
    if(stopFlag) 
    {
      stopAlarm();
      stopFlag = false;
    }
    else if (audioFile.available())
    {
      int sample = audioFile.read(); 
      dacWrite(DAC_OUT, sample);  
    } 
    else 
    {
      repeatAlarm();    
    }
  }
}

void playWAV(String fileName)
{
  // Open the WAV file
  audioFile = SPIFFS.open(fileName, "r"); 
  if (!audioFile) {
    Serial.println("Failed to open file");
    delay(3000);
    return;
  }

  // maybe move this all into if
  sampleTimer = timerBegin(1000000); // TODO try changing to 80
  if(audioFile.available())
  {
    timerAttachInterrupt(sampleTimer, &isr_play_sample);
    Serial.println("interrupt attached");
  }
  timerAlarm(sampleTimer, 62, true, 0); // 1/16000Hz = 62.5us
  audioFile.seek(44); // skip WAV header
}

void triggerAlarm(String alarmFile, int repeats) // wrapper/enum for alarm sounds
{
  Serial.println("Alarm was triggered!");
  repeatCount = repeats - 1;
  playWAV(alarmFile);
}

void stopAlarm()
{
  timerEnd(sampleTimer);  
  audioFile.close();      
  repeatCount = 0;       
  Serial.println("Alarm stopped.");
}

void repeatAlarm()
{
  if(repeatCount)
  {
    Serial.println("Repeating the alarm.");
    repeatCount--;
    audioFile.seek(44);
  }
  else
  {
    stopAlarm();
  }
}
