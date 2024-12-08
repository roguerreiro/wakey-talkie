#include <Adafruit_GFX.h>
#include <Adafruit_GrayOLED.h>
#include <gfxfont.h>
#include <PxMatrix.h>
#include <WiFi.h>
#include <Arduino.h>
#include <FS.h>
#include <SPIFFS.h>
#include "PlayAudio.h"
#include "Alarm.h"
#include "WakeyComm.h"

// hi this is Emma's good fresh code

// Time API Setupp
const char* ssid       = "DukeVisitor";
const char* password   = "";
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -18000; // Eastern Standard Time
const int   daylightOffset_sec = 3600; // observes daylight savings
char the_time[10];
volatile int hour;
volatile int minute;
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

// #define TIMING_PIN 5
#define STOP_BTN 21


void send_message();


PxMATRIX display(32, 32, P_LAT, P_OE, P_A, P_B, P_C, P_D);

// Time Setup
hw_timer_t *displayTimer = NULL;
hw_timer_t *clockTimer = NULL;
hw_timer_t *sampleTimer = NULL;

enum PlayingState : uint8_t
{
  NOT_PLAYING = 0,
  PLAYING_ALARM,
  PLAYING_MSG
};

PlayingState playingState = NOT_PLAYING;

void formatTime();

char *playing_buf = nullptr;        // Define and initialize buffers
char *filling_buf = nullptr;
volatile int playing_buf_size = 0;
volatile int filling_buf_size = 0;
volatile int playing_idx = 0;

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

  if (playing_buf == nullptr || filling_buf == nullptr) 
  {
    Serial.println("Memory allocation failed.");
    return;  // Early return to prevent further errors
  }

  //Rx setup
//  rxSetup();
//  Serial.print("After rxSetup(), isChipConnected()? ");
//  Serial.println(radio.isChipConnected());
  playingState = PLAYING_ALARM;
  sampleTimer = timerBegin(1000000); 
  triggerAlarm(alarmFiles[1], 3, sampleTimer);
}

void loop() 
{
  if(displayFlag)
  {
    display.display(10);
    displayFlag = false;
  }
  if(secondFlag)
  {
    second++;
    display.clearDisplay();
    formatTime();
    display.print(the_time);
    if(checkAlarmTime(hour, minute, am))
    {
      playingState = PLAYING_ALARM;
      triggerAlarm(alarmFiles[alarm_index], 2, sampleTimer);
    }
    secondFlag = false;
  }
  if(filling_buf_size == 0)
  {
    switch(playingState)
    {
      case PLAYING_ALARM:
      {
        fillBuffer(alarmFile, sampleTimer);
        break;
      }
      case PLAYING_MSG:
      {
        
        break;
      }
      default:
      break;
    }
  }
  if(stopFlag)
  {
    playingState = NOT_PLAYING; // means fillBuffer won't be called anymore
    stopAlarm(sampleTimer);
    stopFlag = false;
  }
  if(receivePacket())
  {
    processPacket();
  }
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
    sprintf(the_time, "%d:0%d:%d", hour, minute, second); // TODO: remove seconds
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
