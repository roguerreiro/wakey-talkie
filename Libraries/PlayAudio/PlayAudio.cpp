#include "PlayAudio.h"

File msgFile;

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
  // Serial.println("SB");
  playing_buf_size = filling_buf_size;
  filling_buf_size = 0;
  char *tmp = filling_buf;
  filling_buf = playing_buf;
  playing_buf = tmp;
  playing_idx = 0;
  interrupts();
}

void fillBuffer(File file)
{
  Serial.println("FB");
  if(file.available())
  {
    filling_buf_size = file.readBytes(filling_buf, BUFFER_SIZE);
    // Serial.println(filling_buf_size, DEC);
  }
  else if(playingState == PLAYING_ALARM)
  {
    repeatAlarm();
  }
  else if(playingState == PLAYING_MSG)
  {
    Serial.println("msgFile is over, closing and detaching.");
    playingState = NOT_PLAYING;
    timerDetachInterrupt(sampleTimer);
    timerEnd(sampleTimer);
    msgFile.close();
  }
}

void playMsg()
{
  msgFile = SPIFFS.open("/msg.bin", "r");
  if(!msgFile)
  {
    Serial.println("Failed to open /msg.bin");
    return;
  }
  if(msgFile)
  {
    Serial.println("msgFile is not NULL");
  }
  else
  {
    Serial.println("msgFile is NULL");
  }
    // Print the first 64 bytes
  // Serial.println("Printing the first 64 bytes of /msg.bin:");
  // for (int i = 0; i < 64; i++) {
  //   if (msgFile.available()) {
  //     byte data = msgFile.read();  // Read one byte at a time
  //     Serial.print("Byte ");
  //     Serial.print(i);
  //     Serial.print(": ");
  //     Serial.println(data, HEX);  // Print byte in hexadecimal format
  //   } else {
  //     Serial.println("End of file reached before 64 bytes.");
  //     break;
  //   }
  // }
  if(msgFile.available())
  {
    fillBuffer(msgFile);
    switchBuffers();
    sampleTimer = timerBegin(1000000);
    timerAttachInterrupt(sampleTimer, &isr_play_sample);
    timerAlarm(sampleTimer, 62, true, 0); // 1/16000Hz = 62.5us
    Serial.println("playing msgFile");
  }
  else
  {
    Serial.println("msgFile not available");
  }
}