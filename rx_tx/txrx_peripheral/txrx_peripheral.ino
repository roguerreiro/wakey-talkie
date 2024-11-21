#include <SPI.h>
#include <RF24.h>

RF24 radio(12, 13); // CE and CSN pins

const uint64_t peripheralAddress = 0xF0F0F0F0E1LL; // Peripheral's listening address
const uint64_t hubAddress = 0xF0F0F0F0D2LL; // Address to send responses to the hub

//define pins for the botton, sending and receiving
const int button_pin = 21;
const int send_pin = 16 ;
const int rec_pin = 17;

void setup() {
 Serial.begin(9600);
 pinMode(button_pin, INPUT);
 pinMode(send_pin, OUTPUT);
 pinMode(rec_pin, OUTPUT);
 radio.begin();
 radio.setPALevel(RF24_PA_MAX);
 radio.setChannel(75);
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
    radio.stopListening();
    Serial.print("Message received: ");
    Serial.println(receivedMessage);
    digitalWrite(rec_pin, HIGH);
    delay(100);
    digitalWrite(rec_pin, LOW);
    radio.startListening();
    //Serial.println(radio.isChipConnected());
    }
}

void send_message(){
  radio.stopListening();
  const char response[] = "Hello Hub";
  bool success = radio.write(&response, sizeof(response));

  if(success){
     Serial.println("Message sent successfully");
     digitalWrite(send_pin, HIGH);
     delay(100);
     digitalWrite(send_pin, LOW);
   }

  else{
     Serial.println("Message sending failed");
     digitalWrite(send_pin, LOW);
     delay(5);
   }
   radio.openReadingPipe(1,peripheralAddress);
   radio.startListening(); // Go back to listening mode
   //Serial.println(radio.isChipConnected());
   delay(5);
}


void loop() {

  // send_message();
  if (digitalRead(button_pin)==HIGH){
    send_message();
  }

  else{
    receive_message();
  }
  delay(10);
}