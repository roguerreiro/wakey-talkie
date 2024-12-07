#ifndef PLAY_AUDIO_H
#define PLAY_AUDIO_H

#include <FS.h>

extern char *playing_buf;
extern char *filling_buf;
extern char *tmp;
extern int playing_buf_size = 0;
extern int filling_buf_size = 0;
extern int playing_idx = 0;

void fillBuffer(File file);
void IRAM_ATTR switchBuffers();

#endif // PLAY_AUDIO_H