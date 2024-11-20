#include <SPI.h>

#include <RF24.h>



#define MOSI 23

#define MISO 19

#define CLK 18

#define CE 15

#define CSN 2



RF24 radio(CE, CSN); // CE and CSN pins



const uint64_t peripheralAddress = 0xF0F0F0F0E1LL; // Peripheral's listening address

const uint64_t hubAddress = 0xF0F0F0F0D2LL; // Address to send responses to the hub



void setup() {

 Serial.begin(9600);

 radio.begin();

 radio.setPALevel(RF24_PA_LOW);

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
