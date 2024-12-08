#ifndef MSG_FILE_H
#define MSG_FILE_H

#include <SPIFFS.h>
#include <FS.h>

extern File msgFile;
extern bool msgFileOpen;

void openMsgFile();
void closeMsgFile();
void addToMsgFile(uint8_t *packet, int packet_len);

#endif // MSG_FILE_H