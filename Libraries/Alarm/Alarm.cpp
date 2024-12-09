#include "Alarm.h"

// default values
uint16_t alarm_time = 0; 
uint8_t alarm_index = -1;
File alarmFile;
uint8_t repeatCount = 0;

const char *alarmFiles[] = 
{
    "/hitsdifferent8.wav",
    "/wakeywakey.wav"
};

bool checkAlarmTime(int hour, int minute, bool am)
{
  if(ALARM_MINUTE(alarm_time) == minute)
  {
    if(ALARM_HOUR(alarm_time) == hour)
    {
      if(ALARM_AM(alarm_time) == (am ? 1 : 0))
      {
        return true;
      }
    }
  }
  return false;
}

void triggerAlarm(const char *fileName, int repeats, hw_timer_t *timer)
{
  Serial.println("Alarm was triggered.");
  Serial.println(fileName);
  repeatCount = repeats - 1;
  playWAV(fileName, timer);
}

void repeatAlarm(hw_timer_t *timer)
{
  if(repeatCount)
    {
      Serial.println("Repeating the alarm.");
      repeatCount--;
      alarmFile.seek(44);
    }
    else
    {
      stopAlarm(timer);
    }
}

void stopAlarm(hw_timer_t *timer)
{
  timerDetachInterrupt(timer);
  alarmFile.close();
  repeatCount = 0;
  filling_buf_size = 0;
  playingState = NOT_PLAYING;
  Serial.println("Alarm stopped.");
}

void playWAV(const char *fileName, hw_timer_t *timer)
{
  if (timer == nullptr) {
    Serial.println("Timer is nullptr.");
    return;
  
  }
  Serial.println("In playWAV...");
  Serial.println(fileName);
  // Open the WAV file
  alarmFile = SPIFFS.open(fileName, "r"); 
  if (!alarmFile) {
    Serial.println("Failed to open file"); 
    delay(3000);
    return;
  }
  else
  {
    Serial.println("alarmFile opened.");
  }

  if(alarmFile.available())
  {
    fillBuffer(alarmFile, timer);
    switchBuffers();
    timerAttachInterrupt(timer, &isr_play_sample);
    timerAlarm(timer, 62, true, 0); // 1/16000Hz = 62.5us
    Serial.println("WAV file playing");
  }
  else
  {
    Serial.println("alarmFile not available.");
  }
  
  alarmFile.seek(44); // skip WAV header
}