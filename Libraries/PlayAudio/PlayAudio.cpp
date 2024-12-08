#include "PlayAudio.h"

// char *playing_buf = nullptr;  
// char *filling_buf = nullptr;
// volatile char *tmp = nullptr;
// volatile int playing_buf_size = 0;
// volatile int filling_buf_size = 0;
// volatile int playing_idx = 0;

void IRAM_ATTR isr_play_sample()
{
  dacWrite(DAC_OUT, playing_buf[playing_idx]);
  playing_idx++;
  if(playing_idx == playing_buf_size) 
  {
    switchBuffers();
  }
}

void IRAM_ATTR switchBuffers()
{
  noInterrupts();
  // Serial.println("Switch buffers.");
  playing_buf_size = filling_buf_size;
  filling_buf_size = 0;
  char *tmp = filling_buf;
  filling_buf = playing_buf;
  playing_buf = tmp;
  playing_idx = 0;
  interrupts();
}

void fillBuffer(File file, hw_timer_t *timer)
{
  // Serial.println("fillBuffer");
  if(file.available())
  {
    filling_buf_size = file.readBytes(filling_buf, BUFFER_SIZE);
    // Serial.println(filling_buf_size, DEC);
  }
  else
  {
    repeatAlarm(timer);
  }
}