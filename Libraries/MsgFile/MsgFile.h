#ifndef MSG_FILE_H
#define MSG_FILE_H

#include <SPIFFS.h>
#include <FS.h>

void openMsgFile();
void closeMsgFile();
void addToMsgFile(uint8_t *packet, int packet_len);

extern File msgFile;

#endif