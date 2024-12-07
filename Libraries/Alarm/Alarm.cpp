#include "Alarm.h"

// default values
alarm_time = 0xA148; // 10:20am
alarmFile = "hitsdifferent8.wav";

const char *alarmFiles[] = 
{
    "hitsdifferent8.wav",
    "wakeywakey.wav"
};

bool checkAlarmTime()
{
  if(ALARM_MINUTE(alarm_time) == minute)
  {
    if(ALARM_HOUR(alarm_time) == hour)
    {
      if(ALARM_AM(alarm_time) == am)
      {
        return true;
      }
    }
  }
  return false;
}

void triggerAlarm(const char *fileName, int repeats, hw_timer_t *timer)
{
  repeatCount = repeats - 1;
  playWAV(fileName, timer);
}

void playWAV(const char *fileName, hw_timer_t *timer)
{
  // Open the WAV file
  alarmFile = SPIFFS.open(fileName, "r"); 
  if (!alarmFile) {
    Serial.println("Failed to open file"); 
    delay(3000);
    return;
  }

  if(audioFile.available())
  {
    fillBuffer();
    switchBuffers();
    timer = timerBegin(1000000); 
    timerAttachInterrupt(timer, &isr_play_sample);
    timerAlarm(timer, 62, true, 0); // 1/16000Hz = 62.5us
    Serial.println("WAV file playing");
  }
  
  audioFile.seek(44); // skip WAV header
}