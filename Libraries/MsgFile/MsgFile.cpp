#include "MsgFile.h"

void openMsgFile()
{
    msgFile = SPIFFS.open("/msg.bin", FILE_APPEND);
    if (!msgFile) 
    {
        Serial.println("Failed to open file for writing");
        return;
    }
}

void closeMsgFile() 
{
    if (msgFile) 
    {
        msgFile.close();
        Serial.println("File closed.");
    } 
    else 
    {
        Serial.println("Message file was never open so could not be closed.");
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