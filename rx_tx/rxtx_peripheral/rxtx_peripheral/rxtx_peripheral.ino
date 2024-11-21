#include <SPI.h>
#include <RF24.h>

#define MOSI 23
#define MISO 19
#define CLK 18
#define CE 5
#define CSN 2

SPIClass customSPI (VSPI);
void rxSetup();
RF24 radio(CE, CSN); // CE and CSN pins

const uint64_t peripheralAddress = 0xF0F0F0F0E1LL; // Peripheral's listening address
const uint64_t hubAddress = 0xF0F0F0F0D2LL; // Address to send responses to the hub

void setup() {
 Serial.begin(115200);
 rxSetup();
 delay(5);
}

void receive_message(){
   if (radio.available()) {
    char receivedMessage[32] = "";
    radio.read(&receivedMessage, sizeof(receivedMessage));
    radio.stopListening();
    Serial.print("Message received: ");
    Serial.println(receivedMessage);
    radio.startListening();
    Serial.println(radio.isChipConnected());
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


void loop() {

  // send_message();
  // if (digitalRead(button_pin)==HIGH){
  //   send_message();
  // }
    receive_message();
  delay(10);
}

void rxSetup()
{
  // RX/TX
  customSPI.begin(CLK, MISO, MOSI, CSN); // Ensure CSN is used here
  if (!radio.begin(&customSPI)) {
    Serial.println("Failed to initialize radio");
     while (1); // Halt if initialization fails
  }
  else{
    Serial.println("radio is connected");
  }
  
 radio.setPALevel(RF24_PA_LOW);
 radio.setChannel(75);
 radio.openReadingPipe(1, peripheralAddress);
 radio.openWritingPipe(hubAddress); // Pipe for sending responses back to the hub
 radio.startListening();
 
 if (radio.isChipConnected()){
  Serial.println("Chip is connected");
 }
 else{
  Serial.println("Chip is not connected");
 }
 delay(5);
}