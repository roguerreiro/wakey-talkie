#ifndef MSG_FILE_H
#define MSG_FILE_H

#include <SPIFFS.h>
#include <FS.h>

extern File msgFile;
extern bool msgFileCreated;
extern bool msgFileOpened;
extern bool msgWaiting;
extern uint16_t expireTime;

void listFiles();

void createMsgFile();
void closeMsgFile();
void addToMsgFile(uint8_t *packet, int packet_len);

#endif // MSG_FILE_H