#include "PlayAudio.h"

// char *playing_buf = nullptr;  
// char *filling_buf = nullptr;
// volatile char *tmp = nullptr;
// volatile int playing_buf_size = 0;
// volatile int filling_buf_size = 0;
// volatile int playing_idx = 0;

#define TIMING_PIN 5

extern hw_timer_t *sampleTimer;

void IRAM_ATTR isr_play_sample()
{
  // if(playing_buf_size) // i don't think we should need this and it makes me nervous that i had to add it.
  // {
    dacWrite(DAC_OUT, playing_buf[playing_idx]);
    playing_idx++;
    if(playing_idx == playing_buf_size)  
    {
      switchBuffers();
    }
  // }

}

void IRAM_ATTR switchBuffers()
{
  digitalWrite(TIMING_PIN, HIGH);
  noInterrupts();
  // Serial.println("Switch buffers.");
  playing_buf_size = filling_buf_size;
  filling_buf_size = 0;
  char *tmp = filling_buf;
  filling_buf = playing_buf;
  playing_buf = tmp;
  playing_idx = 0;
  digitalWrite(TIMING_PIN, LOW);
  interrupts();
}

void fillBuffer(File file)
{
  // Serial.println("fillBuffer");
  if(file.available())
  {
    filling_buf_size = file.readBytes(filling_buf, BUFFER_SIZE); 
    Serial.println(filling_buf_size, DEC);

    if(filling_buf_size == 0)
    {
      Serial.println("JUST READ IN ZERO BYTES TO FILL BUFFER.");
    }
  }
  else
  {

    Serial.println("-------------------------else--------------------------");
    Serial.print("playing_idx: ");
    Serial.println(playing_idx, DEC);
    Serial.print("playing_buf_size: ");
    Serial.println(playing_buf_size, DEC); // this is being 0 with a nonzero index

    // playing_buf_size = 0;
    if(playingState == PLAYING_ALARM) // MIGHT CHANGE TO 1 or whatever instead
    {
      repeatAlarm();
    }
    else if(playingState == PLAYING_MSG)
    {
      playingState = NOT_PLAYING;
      Serial.println("else playing_msg > closing msgFile");
      timerDetachInterrupt(sampleTimer);
      msgFile.close();
      filling_buf_size = 0;
    }
  }
}

void playMsg(hw_timer_t *timer) // probably doesn't need to take in a timer at all
{
  // if (timer == nullptr) {
  //   Serial.println("Timer is nullptr.");
  //   return;
  
  // }
  Serial.println("In playMsg...");
  // Open the WAV file
  msgFile = SPIFFS.open("/msg.bin", "r"); 
  if (!msgFile) {
    Serial.println("Failed to open file"); 
    delay(3000);
    return;
  }
  else
  {
    Serial.println("msgFile opened.");
  }

  if(msgFile.available())
  {
    fillBuffer(msgFile);
    switchBuffers();
    sampleTimer = timerBegin(1000000);
    timerAttachInterrupt(sampleTimer, &isr_play_sample);
    timerAlarm(sampleTimer, 62, true, 0); // 1/16000Hz = 62.5us
    Serial.println("Msg file playing");
  }
  else
  {
    Serial.println("msgFile not available.");
  }
}