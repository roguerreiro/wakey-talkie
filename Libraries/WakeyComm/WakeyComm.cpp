#include "WakeyComm.h"

const uint64_t peripheralAddress = 0xF0F0F0F0E1LL; // Peripheral's listening address
const uint64_t hubAddress = 0xF0F0F0F0D2LL; // Address to send responses to the hub

SPIClass customSPI(VSPI);
RF24 radio(CE, CSN);

char packet[32] = "";

void rxSetup() // maybe have this try again if it fails rather than while(1)
{
  customSPI.begin(CLK, MISO, MOSI, CSN); // Ensure CSN is used here
  if (!radio.begin(&customSPI)) {
    Serial.println("Failed to initialize radio"); // maybe redo?
    return;
  }
  else
  {
    Serial.println("Radio is connected");
  }
  
 radio.setPALevel(RF24_PA_HIGH);
 radio.setChannel(75);
 radio.openReadingPipe(1, peripheralAddress);
 radio.openWritingPipe(hubAddress); // Pipe for sending responses back to the hub
 radio.startListening();
 
 if(radio.isChipConnected())
 {
  Serial.println("Chip is connected.");
 }
 else
 {
  Serial.println("Chip is not connected");
 }
 delay(5);
}

bool receivePacket()
{
   if (radio.available()) 
   {
    uint8_t bytes = radio.getPayloadSize();
    radio.read(&packet, sizeof(packet));
    radio.stopListening();
    Serial.print("Message received: ");
    for(int i=0; i<32; i++)
    {
      Serial.print(packet[i], HEX);
      Serial.print(" ");
    }
    Serial.print("\n");
    // Serial.println(receivedMessage);
    radio.startListening();
    // Serial.println(radio.isChipConnected());
    return true;
   }
   return false;
}

void processPacket()
{
  if (packet[0] < MAX_OPCODE) 
  {  
    Opcode opcode = (Opcode)packet[0];  // Cast the first byte to an Opcode enum
    switch (opcode) 
    {
      case SET_ALARM:
          Serial.println("SET_ALARM");
          alarm_time = ((uint16_t)packet[1] << 8) | packet[2];
          Serial.print("alarm_time set to: 0x");
          Serial.println(alarm_time, HEX); 
          break;
      case SET_EXPIRATION:
          Serial.println("SET_EXPIRATION");
          break;
      case MSG_PACKET:
          Serial.println("MSG_PACKET");
          if(!msgFileCreated)
          {
            createMsgFile();
            addToMsgFile((uint8_t *)&packet[1], 31);
          }
          else
          {
            addToMsgFile((uint8_t *)&packet[1], 31);
          }
          break;
      case MSG_COMPLETE:
          Serial.println("MSG_COMPLETE");
          if(playingState == NOT_PLAYING)
          {
            Serial.println("Closing msgFile");
            closeMsgFile();
            if(isPersonActive()) // TODO: fix
            {
              // person is here > play message immediately
              playingState = PLAYING_MSG;
              playMsg();
            }
            else
            {
              msgWaiting = true;
            }
          }
          else
          {
            Serial.println("something is wrong");
          }
          break;
      case INTERCOM_STATUS:
          Serial.println("INTERCOM_STATUS");
          break;
      default:
      Serial.println("PING");
          break;
    }
  } 
  else 
  {
      Serial.println("Invalid opcode.");
  }
}