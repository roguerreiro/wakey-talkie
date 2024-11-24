#include <SPI.h>
#include <RF24.h>

#define MOSI 23
#define MISO 19
#define CLK 18
#define CE 15
#define CSN 2
#define DAC_OUT 25
#define BUFFER_SIZE 1024

#define TIMING_PIN 4

RF24 radio(CE, CSN); // CE and CSN pins

const uint64_t peripheralAddress = 0xF0F0F0F0E1LL; // Peripheral's listening address
const uint64_t hubAddress = 0xF0F0F0F0D2LL; // Address to send responses to the hub
int sample = 0;

uint8_t *filling_buf;
uint8_t *playing_buf;
uint8_t filling_buf_size = 0;
uint8_t playing_buf_size = 0;
uint8_t playing_idx = 0;
uint8_t *receiving_buf;

hw_timer_t *sampleTimer = NULL;


#define RECEIVE_BUFFER_SIZE 32

void fillBuffer(uint8_t *msg, uint8_t msg_len);
void switchBuffers();

void IRAM_ATTR isr_play_sample()
{
//  digitalWrite(TIMING_PIN, 1);
  dacWrite(DAC_OUT, playing_buf[playing_idx]);
  playing_idx++;
  if(playing_idx == playing_buf_size)
  {
    switchBuffers();
  }
//  digitalWrite(TIMING_PIN, 0);
}


void setup() {
 Serial.begin(9600);

  pinMode(TIMING_PIN, OUTPUT);

  filling_buf = (uint8_t *)malloc(BUFFER_SIZE);
  playing_buf = (uint8_t *)malloc(BUFFER_SIZE);
  receiving_buf = (uint8_t*) malloc(RECEIVE_BUFFER_SIZE);
 
  radio.begin();
  radio.setPALevel(RF24_PA_LOW);
  radio.setChannel(75);
  radio.setAutoAck(false);
  radio.openReadingPipe(1, peripheralAddress);
  radio.openWritingPipe(hubAddress); // Pipe for sending responses back to the hub
  radio.startListening();
  Serial.println(radio.isChipConnected());
  delay(5);

  memset(playing_buf, 0, BUFFER_SIZE);
  playing_buf_size = BUFFER_SIZE;

  sampleTimer = timerBegin(1000000); 
  timerAttachInterrupt(sampleTimer, &isr_play_sample);
  timerAlarm(sampleTimer, 62, true, 0); // 1/16000Hz = 62.5us
  Serial.println("Timer started");
}

//void receive_message(){
//   if (radio.available()) {
//    uint8_t receivedMessage[RECEIVE_BUFFER_SIZE] = "";
//    radio.read(&receivedMessage, sizeof(receivedMessage));
//    fillBuffer(receivedMessage, RECEIVE_BUFFER_SIZE);
//  }
//}

bool receive_audio(uint8_t* buffie) {
  if (radio.available()) {
    radio.read(buffie, RECEIVE_BUFFER_SIZE);
    return true;
  }
  else {
    return false;
//    memset(buffie, 0, RECEIVE_BUFFER_SIZE);
//    Serial.println("Read garbage 0s");
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
    if(receive_audio(receiving_buf))
    {
      receive_audio(receiving_buf);
      fillBuffer(receiving_buf, RECEIVE_BUFFER_SIZE);
    }
    else
    {
      // put in a few zeros or something
    }
  }
}

void fillBuffer(uint8_t *msg, uint8_t msg_len)
{
//  digitalWrite(TIMING_PIN, 1);
  memcpy(filling_buf, msg, msg_len);
  filling_buf_size = msg_len;
//  Serial.print("Buffer: ");
//  for(int i=0; i<32; i++)
//  {
//    Serial.print(filling_buf[i]);
//    Serial.print(", ");
//  }
//  Serial.println("\n");
//  memcpy(filling_buf + filling_buf_size, msg, msg_len);
//  filling_buf_size = filling_buf_size + msg_len;
//  digitalWrite(TIMING_PIN, 0);
}

void IRAM_ATTR switchBuffers()
{
  if(filling_buf_size == 0)
  {
    playing_idx = playing_idx - 1;
  }
  else
  {
    playing_buf_size = filling_buf_size;
    filling_buf_size = 0;
    uint8_t *tmp = filling_buf;
    filling_buf = playing_buf;
    playing_buf = tmp;
    playing_idx = 0;
  }
}
