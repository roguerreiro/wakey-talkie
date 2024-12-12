#include <Adafruit_GFX.h>
#include <Adafruit_GrayOLED.h>
#include <gfxfont.h>
#include <PxMatrix.h>
#include <WiFi.h>
#include <Arduino.h>
#include <FS.h>
#include <SPIFFS.h>
#include "Sensing.h"
#include "PlayAudio.h"
#include "Alarm.h"
#include "WakeyComm.h"
#include "MsgFile.h"

/*
 * TODO fix stopping starting again.
 * 
 * customize waky1 and wakey2?
 * do we want to customize repeats or have a default?
 */

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
volatile bool msgFlag = false;
volatile bool senseFlag = false;

// LED Matrix Display Pins
#define P_LAT 12
#define P_A 32
#define P_B 33
#define P_C 26
#define P_D 27
#define P_OE 22
#define RANDOM_PIN 36

// #define TIMING_PIN 5
#define STOP_BTN 21 // NEEDS TO BE SET, THIS IS TEMP
#define MSG_BTN 4 

void send_message();

PxMATRIX display(32, 32, P_LAT, P_OE, P_A, P_B, P_C, P_D);

// Time Setup
hw_timer_t *displayTimer = NULL;
hw_timer_t *clockTimer = NULL;
hw_timer_t *sampleTimer = NULL;


PlayingState playingState = NOT_PLAYING;

bool formatTime();

char *playing_buf = nullptr;        // Define and initialize buffers
char *filling_buf = nullptr;
volatile int playing_buf_size = 0;
volatile int filling_buf_size = 0;
volatile int playing_idx = 0;

uint16_t randomColor()
{
  return random(65535);
}

void IRAM_ATTR isr_msg_pressed()
{
  msgFlag = true;
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
  senseFlag = true;
}

void setup() 
{
  Serial.begin(115200);

  // Enable button interrupts
  pinMode(MSG_BTN, INPUT);
  attachInterrupt(MSG_BTN, isr_msg_pressed, RISING);
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
  if(timeinfo.tm_hour < 12)
  {
    am = true;
  }
  if(hour == 0)
  {
    hour = 12; 
    Serial.println("this ran.");
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

  listFiles();

  playing_buf = (char *)malloc(BUFFER_SIZE);
  filling_buf = (char *)malloc(BUFFER_SIZE);

  if (playing_buf == nullptr || filling_buf == nullptr) 
  {
    Serial.println("Memory allocation failed.");
    return;  
  }

  //Rx setup
  rxSetup();

  //sensor setup
  senseSetup();
//  Serial.print("After rxSetup(), isChipConnected()? ");
//  Serial.println(radio.isChipConnected());
//  playingState = PLAYING_ALARM;
//  sampleTimer = timerBegin(1000000); 
//  triggerAlarm(alarmFiles[0], 3, sampleTimer);
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
    if(formatTime())
    {
      display.setTextColor(randomColor()); // temporary, might take out
      display.clearDisplay();
      display.print(the_time);
    }
    if(msgWaiting)
    {
      display.drawRect(0,0, 32, 32, 0xFFFF);
    }
    // TODO: also check expiration date stuff
    
    if(playingState != 1) // might not need this check anymore 
    {
      if(checkAlarmTime(hour, minute, am))
      {
        alarm_time = 0;
        Serial.print("playingState: ");
        Serial.println(playingState, DEC);
        Serial.println("alarm - wake up!");
        playingState = PLAYING_ALARM;
        sampleTimer = timerBegin(1000000);
        triggerAlarm(alarmFiles[0], 99, sampleTimer); // todo change to infinite
      }
    }
    secondFlag = false;
  }

  if (senseFlag)
  {
//    bool currentAvail = isAvailable();
//    bool currentAct = isActive();
//
//    Serial.print("currentAvail: ");
//    Serial.println(currentAvail);
//    Serial.print("currentAct: ");
//    Serial.println(currentAct);
    
    updateSenseState(isActive(), isAvailable());
    senseFlag = false;
//    Serial.print("available state:");
//    Serial.println(availHistory, BIN);
//    Serial.print("active state: ");
//    Serial.println(activeHistory, BIN);
//    uint32_t random = 1024;
//    Serial.print("testing: ");
//    Serial.println(random, BIN);
  }
  
  if(filling_buf_size == 0)
  {
    switch(playingState)
    {
      case PLAYING_ALARM:
      {
        fillBuffer(alarmFile);
        break;
      }
      case PLAYING_MSG:
      {
        fillBuffer(msgFile);
        break;
      }
      default:
      break;
    }
  }
  if(stopFlag) 
  {
    if(playingState == PLAYING_ALARM)
    {
      playingState = NOT_PLAYING; 
      stopAlarm();
      stopFlag = false;
    }
  }
  if(msgFlag)
  {
    if(msgWaiting)
    {
      setCursorFromTime();
      display.clearDisplay();
      display.print(the_time);
      Serial.println("Message playback initiated");
      msgWaiting = false;
      playingState = PLAYING_MSG;
      playMsg();
    }
    msgFlag = false;
  }
  if(receivePacket())
  {
    processPacket();
  }
}

/* returns true if time changed */
bool formatTime()
{
  bool ret = false;
  if(second == 60)
  {
    second = 0;
    minute++;
    ret = true;
    if(minute == 60)
    {
      minute = 0;
      hour++;
      if(hour == 12)
      {
        am = !am;
      }
      if(hour == 13)
      {
        hour = 1;
      }
    }
  }
  
  // add leading zero for single digit minutes
  if(minute < 10)
  {
    sprintf(the_time, "%d:0%d", hour, minute); // TODO: remove seconds
  }
  else
  {
    sprintf(the_time, "%d:%d", hour, minute);
  }

  setCursorFromTime();
  return ret;
}

void setCursorFromTime()
{
  if(hour > 9)
  {
    display.setCursor(1 , 12);
  }
  else
  {
    display.setCursor(5 , 12);
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
   radio.openReadingPipe(1, peripheralAddress);
   radio.startListening(); // Go back to listening mode
   Serial.println(radio.isChipConnected());
   delay(5);
}
