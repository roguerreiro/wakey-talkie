#ifndef PLAY_AUDIO_H
#define PLAY_AUDIO_H

#include <FS.h>
#include <Alarm.h>
#include "PlayAudio.h"

#define DAC_OUT 25
#define BUFFER_SIZE 1024


enum PlayingState : uint8_t
{
  NOT_PLAYING = 0,
  PLAYING_ALARM,
  PLAYING_MSG
};

extern PlayingState playingState;

extern char *playing_buf;
extern char *filling_buf;
extern hw_timer_t *sampleTimer;

extern volatile int playing_buf_size;
extern volatile int filling_buf_size;
extern volatile int playing_idx;

void IRAM_ATTR isr_play_sample();
void IRAM_ATTR switchBuffers();
void fillBuffer(File file);

void playMsg();

#endif // PLAY_AUDIO_H