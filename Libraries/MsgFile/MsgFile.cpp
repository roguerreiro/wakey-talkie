#include "MsgFile.h"

File msgFile;
bool msgFileOpen = false;
uint16_t expireTime = 0;

void openMsgFile()
{
    msgFile = SPIFFS.open("/msg.bin", FILE_APPEND);
    msgFileOpen = true;
    if (!msgFile) 
    {
        Serial.println("Failed to open file for writing");
        return;
    }

    // Check if the file exists
    if (SPIFFS.exists("/msg.bin")) {
        Serial.println("File /msg.bin exists!");
    } else {
        Serial.println("File /msg.bin does not exist. ------------------------");
    }
}

void closeMsgFile() 
{
    if (msgFile) 
    {
        msgFile.close();
        msgFileOpen = false;
        Serial.println("File closed.");
    } 
    else 
    {
        Serial.println("Message file was never open so could not be closed.");
    }
    // Check if the file exists
    if (SPIFFS.exists("/msg.bin")) {
        Serial.println("File /msg.bin exists!");
    } else {
        Serial.println("File /msg.bin does not exist. ------------------------");
    }
}

void addToMsgFile(uint8_t *packet, int packet_len)
{
    if (!msgFile) 
    {
        Serial.println("Error: File not open for writing.");
        return;
    }
    int bytesWritten = msgFile.write(packet, packet_len);
    if(bytesWritten != packet_len)
    {
        Serial.println("Failed to write entire packet.");
    }
}