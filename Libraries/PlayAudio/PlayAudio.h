#ifndef PLAY_AUDIO_H
#define PLAY_AUDIO_H

#include <FS.h>
#include <Alarm.h>

#define DAC_OUT 25
#define BUFFER_SIZE 1024

extern char *playing_buf;
extern char *filling_buf;
// volatile extern char *tmp;
extern volatile int playing_buf_size;
extern volatile int filling_buf_size;
extern volatile int playing_idx;



void IRAM_ATTR isr_play_sample();
void IRAM_ATTR switchBuffers();
void fillBuffer(File file, hw_timer_t *timer);

#endif // PLAY_AUDIO_H