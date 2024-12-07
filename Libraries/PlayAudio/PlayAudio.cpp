#include "PlayAudio.h"

void IRAM_ATTR switchBuffers()
{
  playing_buf_size = filling_buf_size;
  filling_buf_size = 0;
  char *tmp = filling_buf;
  filling_buf = playing_buf;
  playing_buf = tmp;
  playing_idx = 0;
}

void fillBuffer(File file)
{
  Serial.println("fillBuffer");
  if(file.available())
  {
    filling_buf_size = file.readBytes(filling_buf, BUFFER_SIZE);
    Serial.println(filling_buf_size, DEC);
  }
  else
  {
    repeatAlarm();
  }
}