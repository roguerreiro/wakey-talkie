#include "MsgFile.h"

// File msgFile;
bool msgFileCreated = false;
bool msgFileOpened = false;
uint16_t expireTime = 0;

void createMsgFile()
{
    msgFile = SPIFFS.open("/msg.bin", FILE_WRITE);
    msgFileCreated = true;
    if (!msgFile) 
    {
        Serial.println("Failed to open file for writing");
        return;
    }
    if (SPIFFS.exists("/msg.bin")) {
        Serial.println("File /msg.bin exists!");
    } else {
        Serial.println("File /msg.bin does not exist!");
    }
}

void listFiles() {
    Serial.println("Called listFiles");
  File root = SPIFFS.open("/");  // Open root directory
  if (!root) {
    Serial.println("Failed to open directory");
    return;
  }

  // Check if it's a directory
  if (!root.isDirectory()) {
    Serial.println("Not a directory!");
    return;
  }

  // Iterate through the files
  root.rewindDirectory();  // Rewind to the start of the directory listing
  while (true) {
    File entry = root.openNextFile();  // Get the next file or directory
    if (!entry) {
      break;  // No more files or directories
    }

    // Check if the entry is a directory or a file
    if (entry.isDirectory()) {
      Serial.print("Dir: ");
    } else {
      Serial.print("File: ");
      Serial.print("Size: ");
      Serial.print(entry.size());  // Print the file size in bytes
      Serial.print(" bytes - ");
    }

    // Print the name of the file or directory
    Serial.println(entry.name());
    entry.close();  // Close the entry after use
  }
}

void closeMsgFile() 
{
    if (msgFile) 
    {
        msgFile.close();
        msgFileCreated = false;
        msgFileOpened = false;
        Serial.println("File closed.");
    } 
    else 
    {
        Serial.println("Message file was never open so could not be closed.");
    }
    listFiles();
}

void addToMsgFile(uint8_t *packet, int packet_len)
{
    if(!msgFileOpened)
    {
        msgFile = SPIFFS.open("/msg.bin", FILE_APPEND); // don't want to do this every time
        msgFileOpened = true;
    }
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