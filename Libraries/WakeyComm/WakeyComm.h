#ifndef WAKEY_COMM_H
#define WAKEY_COMM_H

#include <cstdint>
#include <SPI.h>
#include <RF24.h>

#include "Alarm.h"
#include "MsgFile.h"

// Receiver/Transmitter
#define CE 2
#define CSN 5
#define CLK 18
#define MOSI 23
#define MISO 19

enum Opcode : uint8_t {
    // Talkie > Wakey
    SET_ALARM,
    SET_EXPIRATION,
    MSG_PACKET,
    MSG_COMPLETE,

    // Wakey > Talkie
    INTERCOM_STATUS,
    MAX_OPCODE
};

extern const uint64_t peripheralAddress;
extern const uint64_t hubAddress;
extern char packet[32];

extern SPIClass customSPI;  
extern RF24 radio; 

void rxSetup();

/* 
if radio has an available packet it is read into packet
return true if packet was read
*/
bool receivePacket();
void processPacket();

#endif // WAKEY_COMM_H