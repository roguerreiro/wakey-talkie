/*********
  Rui Santos
  Complete project details at https://randomnerdtutorials.com  
*********/

#include "SPIFFS.h"
 
void setup() {
  Serial.begin(9600);
  
  Serial.println("Hello, World!");
 
  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  else
  {
    Serial.println("SPIFFs began");
  }
  
  File file = SPIFFS.open("/text_example.txt", "r");
  if(!file){
    Serial.println("Failed to open file for reading");
    return;
  }
  else
  {
    Serial.println("File opened");
  }

  file.seek(0);
  Serial.print("File available? ");
  Serial.println(file.available());
  Serial.print("readString: ");
  Serial.println(file.readString());
//  while(file.available()){
//    Serial.println("inside while");
//    Serial.print(file.read());
//    Serial.write(file.read());
//  }
  file.close();
}
 
void loop() {
  Serial.println("loop");
  delay(2000);
}
