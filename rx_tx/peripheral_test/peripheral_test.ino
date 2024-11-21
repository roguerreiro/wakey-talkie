#include <SPI.h>
#include <RF24.h>

#define MOSI 23
#define MISO 19
#define CLK 18
#define CE 15
#define CSN 2
#define DAC_OUT 25
#define BUFFER_SIZE 64

RF24 radio(CE, CSN); // CE and CSN pins

const uint64_t peripheralAddress = 0xF0F0F0F0E1LL; // Peripheral's listening address
const uint64_t hubAddress = 0xF0F0F0F0D2LL; // Address to send responses to the hub
int sample = 0;

uint8_t *filling_buf;
uint8_t *playing_buf;
uint8_t filling_buf_size;
uint8_t playing_buf_size;
uint8_t playing_idx;

void fillBuffer();
void switchBuffers();

void IRAM_ATTR isr_play_sample()
{
  dacWrite(DAC_OUT, playing_buf[playing_idx]);
  playing_idx++;
  if(playing_idx == playing_buf_size)
  {
    switchBuffers();
  }
}


void setup() {
 Serial.begin(9600);

  filling_buf = (uint8_t *)malloc(BUFFER_SIZE);
  playing_buf = (uint8_t *)malloc(BUFFER_SIZE);
 
 radio.begin();
 radio.setPALevel(RF24_PA_LOW);
 radio.setChannel(75);
 radio.setAutoAck(false);
 radio.openReadingPipe(1, peripheralAddress);
 radio.openWritingPipe(hubAddress); // Pipe for sending responses back to the hub
 radio.startListening();
 Serial.println(radio.isChipConnected());
 delay(5);
}

void receive_message(){
   if (radio.available()) {
    char receivedMessage[32] = "";
    radio.read(&receivedMessage, sizeof(receivedMessage));
    fillBuffer(receivedMessage, sizeof(receivedMessage));
  }
}

void send_message(){
  radio.stopListening();
  const char response[] = "Hello Hub";
  bool success = radio.write(&response, sizeof(response));
  if(success){
     Serial.println("Message sent successfully");
   }

  else{
     Serial.println("Message sending failed");
     delay(5);
   }
   radio.openReadingPipe(1,peripheralAddress);
   radio.startListening(); // Go back to listening mode
   Serial.println(radio.isChipConnected());
   delay(5);
}

void loop() 
{
  
  if(filling_buf_size == 0)
  {
    
    // call function that receives a packet and returns the pointer and the length
    fillBuffer();
  }
}

void fillBuffer(uint8_t *msg, uint8_t msg_len)
{
  memcpy(filling_buf, msg, msg_len);
  filling_buf_size = msg_len;
}

void IRAM_ATTR switchBuffers()
{
  playing_buf_size = filling_buf_size;
  filling_buf_size = 0;
  uint8_t *tmp = filling_buf;
  filling_buf = playing_buf;
  playing_buf = tmp;
  playing_idx = 0;
}
