#include <Adafruit_GFX.h>
#include <Adafruit_GrayOLED.h>
#include <gfxfont.h>
#include <PxMatrix.h>
#include <WiFi.h>
#include <Arduino.h>
#include <FS.h>
#include <SPIFFS.h>
#include <SPI.h>
#include <RF24.h>

#include <Alarm.h>

// hi this is Emma's good fresh code

// Time API Setup
const char* ssid       = "DukeVisitor";
const char* password   = "";
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -18000; // Eastern Standard Time
const int   daylightOffset_sec = 3600; // observes daylight savings
char the_time[10];
volatile int minute;
volatile int hour;
volatile int second;
bool am;

// ISR Flags
volatile bool displayFlag = false;
volatile bool secondFlag = false;
volatile bool stopFlag = false;

// LED Matrix Display Pins
#define P_LAT 12
#define P_A 32
#define P_B 33
#define P_C 26
#define P_D 27
#define P_OE 22
#define RANDOM_PIN 34

// Speaker
#define DAC_OUT 25

// #define TIMING_PIN 5
#define STOP_BTN 21

// Receiver/Transmitter
#define CE 2
#define CSN 5
#define CLK 18
#define MOSI 23
#define MISO 19

SPIClass customSPI (VSPI);
RF24 radio(CE, CSN);

const uint64_t peripheralAddress = 0xF0F0F0F0E1LL; // Peripheral's listening address
const uint64_t hubAddress = 0xF0F0F0F0D2LL; // Address to send responses to the hub
void rxSetup();
void send_message();
void receive_message();

PxMATRIX display(32, 32, P_LAT, P_OE, P_A, P_B, P_C, P_D);

// Time Setup
hw_timer_t *displayTimer = NULL;
hw_timer_t *clockTimer = NULL;
hw_timer_t *sampleTimer = NULL;

#define BUFFER_SIZE 1024

char *playing_buf;
char *filling_buf;
char *tmp;
int playing_buf_size = 0;
int filling_buf_size = 0;
int playing_idx = 0;
int sample;

void formatTime();
void fillBuffer();
void switchBuffers();

void stopAlarm();
void repeatAlarm();

uint16_t randomColor()
{
  return random(65535);
}

void IRAM_ATTR isr_stop_pressed()
{
  stopFlag = true;
}

void IRAM_ATTR isr_display_updater()
{
  displayFlag = true;
}

void IRAM_ATTR isr_second_passed()
{
  secondFlag = true;
}

void IRAM_ATTR isr_play_sample()
{
  dacWrite(DAC_OUT, playing_buf[playing_idx]);
  playing_idx++;
  if(playing_idx == playing_buf_size) 
  {
    switchBuffers();
  }
}

void setup() 
{
  Serial.begin(115200);

  // Enable button interrupt
  pinMode(STOP_BTN, INPUT);
  attachInterrupt(STOP_BTN, isr_stop_pressed, RISING);

  // connect to WiFi
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.println(" CONNECTED");

  // Initialize Time API 
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  // initialize random seed to disconnected analog pin input
  pinMode(RANDOM_PIN, INPUT);
  randomSeed(analogRead(RANDOM_PIN));
  
  display.begin(16); 
  display.flushDisplay();

  displayTimer = timerBegin(1000000); 
  timerAttachInterrupt(displayTimer, &isr_display_updater);
  timerAlarm(displayTimer, 5000, true, 0); 

  clockTimer = timerBegin(1000000);
  timerAttachInterrupt(clockTimer, &isr_second_passed);
  timerAlarm(clockTimer, 1000000, true, 0);

  display.setTextColor(randomColor());
  display.setTextSize(1);

  struct tm timeinfo;
  if(!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain time");
    return;
  }

  // grab time from API
  minute = timeinfo.tm_min;
  hour = timeinfo.tm_hour % 12;
  if(hour == 0)
  {
    hour = 12; 
  }
  second = timeinfo.tm_sec;

  formatTime();
  display.clearDisplay(); 
  display.print(the_time);

  // Initialize SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS mount failed");
    delay(3000);
    return;
  }

  playing_buf = (char *)malloc(BUFFER_SIZE);
  filling_buf = (char *)malloc(BUFFER_SIZE);

  //Rx setup
  rxSetup();
  Serial.print("After rxSetup(), isChipConnected()? ");
  Serial.println(radio.isChipConnected());

  triggerAlarm("/wakeywakey.wav", 3);
}

void loop() 
{
  if(displayFlag)
  {
    // digitalWrite(TIMING_PIN, 1);
    display.display(10);
    displayFlag = false;
    // digitalWrite(TIMING_PIN, 0);
  }
  if(secondFlag)
  {
    second++;
    display.clearDisplay();
    formatTime();
    display.print(the_time);
    if(checkAlarmTime())
    {
      triggerAlarm(alarmFiles[alarm_index], 2);
    }
    secondFlag = false;
  }
  if(filling_buf_size == 0)
  {
    fillBuffer();
  }
  if(stopFlag)
  {
    stopAlarm();
    stopFlag = false;
  }
  receive_message();
}


void formatTime()
{
  if(second == 60)
  {
    second = 0;
    minute++;
    if(minute == 60)
    {
      minute = 0;
      hour++;
      if(hour == 13)
      {
        hour = 1;
      }
    }
  }
  
  // add leading zero for single digit minutes
  if(minute < 10)
  {
    sprintf(the_time, "%d:0%d:%d", hour, minute, second);
  }
  else
  {
    sprintf(the_time, "%d:%d:%d", hour, minute, second);
  }

  // adjust cursor based on hour digits
  if(hour > 9)
  {
    display.setCursor(1 , 10);
  }
  else
  {
    display.setCursor(5 , 10);
  }
}

void stopAlarm()
{
  timerDetachInterrupt(sampleTimer);
  audioFile.close();
  repeatCount = 0;
  filling_buf_size = -1;
  Serial.println("Alarm stopped.");
}

void repeatAlarm()
{
  if(repeatCount)
    {
      Serial.println("Repeating the alarm.");
      repeatCount--;
      audioFile.seek(44);
    }
    else
    {
      stopAlarm();
    }
}

void fillBuffer()
{
  Serial.println("fillBuffer");
  if(audioFile.available())
  {
    filling_buf_size = audioFile.readBytes(filling_buf, BUFFER_SIZE);
    Serial.println(filling_buf_size, DEC);
  }
  else
  {
    repeatAlarm();
  }
}

void IRAM_ATTR switchBuffers()
{
  playing_buf_size = filling_buf_size;
  filling_buf_size = 0;
  char *tmp = filling_buf;
  filling_buf = playing_buf;
  playing_buf = tmp;
  playing_idx = 0;
}

void receive_message()
{
  //SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
   if (radio.available()) 
   {
    Serial.println("radio.available() return true.");
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

void rxSetup()
{
  // RX/TX
  customSPI.begin(CLK, MISO, MOSI, CSN); // Ensure CSN is used here
  if (!radio.begin(&customSPI)) {
    Serial.println("Failed to initialize radio");
     while (1); // Halt if initialization fails
  }
  else
  {
    Serial.println("Radio is connected");
  }
  
 radio.setPALevel(RF24_PA_LOW);
 radio.setChannel(75);
 radio.openReadingPipe(1, peripheralAddress);
 radio.openWritingPipe(hubAddress); // Pipe for sending responses back to the hub
 radio.startListening();
 
 if (radio.isChipConnected())
 {
  Serial.println("Chip is connected.");
 }
 else
 {
  Serial.println("Chip is not connected");
 }
 delay(5);
}
