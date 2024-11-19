/*
 * bugs:
 * 
 * - display.display cannot run in ISR - too slow! causes semaphore bug
 * - alarm triggering causes timer register error becuase no free timer > 
 *      need to address this and shut down all timers
 *      
 * - alarm seems to be retriggering before alarm ends!
 */
#define TIMING_PIN 27
 
bool alarmEnable = true; // temp workaround
// to make sure trigger doesn't get called repeatedly within the second

#include <Adafruit_GrayOLED.h>
#include <gfxfont.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SPITFT.h>
#include <Adafruit_SPITFT_Macros.h>
#include <PxMatrix.h>
#include <WiFi.h>
#include <Arduino.h>
#include <FS.h>
#include <SPIFFS.h>

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

// ISR Flags
volatile bool sampleFlag = false;
volatile bool stopFlag = false;
volatile bool updateDisplayFlag = false;

// maybe change to defines
const int STOP_BTN = 32;

#define DAC_OUT 25

hw_timer_t *sampleTimer = NULL;
File audioFile; 

// for now, assume only one alarm choice
int repeatCount = 0;

// LED Matrix Display
#define P_LAT 22
#define P_A 19
#define P_B 23
#define P_C 18
#define P_D 5
#define P_OE 15
PxMATRIX display(32,32,P_LAT, P_OE,P_A,P_B,P_C,P_D);

#define RANDOM_PIN 34

// Time Setup
hw_timer_t *displayTimer = NULL;
hw_timer_t *clockTimer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

volatile bool second_flag;

uint16_t randomColor()
{
  return random(65535);
}

//void IRAM_ATTR isr_stop_alarm()
//{
//  stopFlag = true;
//}
//
void IRAM_ATTR isr_play_sample()
{
  sampleFlag = true;
  digitalWrite(TIMING_PIN, 1);
  int sample = audioFile.read(); 
  dacWrite(DAC_OUT, sample);
  digitalWrite(TIMING_PIN, 0);
}

//void IRAM_ATTR isr_second_passed()
//{
//  portENTER_CRITICAL_ISR(&timerMux);
//  second_flag = 1;
//  portEXIT_CRITICAL_ISR(&timerMux);
//}

void IRAM_ATTR isr_display_updater()
{
  updateDisplayFlag = true;
//  portENTER_CRITICAL_ISR(&timerMux);
//  display.display(70); 
//  portEXIT_CRITICAL_ISR(&timerMux);
}

void formatTime();
void triggerAlarm(String alarmFile, int repeats);
void playWAV(String fileName);
void stopAlarm();
void repeatAlarm();

void setup() 
{
  Serial.begin(115200);

  pinMode(TIMING_PIN, OUTPUT);

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

//  clockTimer = timerBegin(1000000);
//  timerAttachInterrupt(clockTimer, &isr_second_passed);
//  timerAlarm(clockTimer, 1000000, true, 0);
  
  displayTimer = timerBegin(1000000); // TODO try changing to 80
  timerAttachInterrupt(displayTimer, &isr_display_updater);
  timerAlarm(displayTimer, 4000, true, 0);

  display.setTextColor(randomColor());
  display.setTextSize(1);

  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
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

  // Enable button interrupt
//  pinMode(STOP_BTN, INPUT);
//  attachInterrupt(STOP_BTN, isr_stop_alarm, RISING);
  
  // Initialize SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS mount failed");
    delay(3000);
    return;
  }

//  triggerAlarm("/hitsdifferent8.wav", 3); // trying this to see if it fixes
  // timer register bug > IT DID
}

void loop() {
//  if(sampleFlag) // time to play a sample
//  {
//    sampleFlag = false; 
//    if(stopFlag) 
//    {
//      stopAlarm();
//      stopFlag = false;
//    }
//    else if (audioFile.available())
//    {
////      digitalWrite(TIMING_PIN, 1);
//      int sample = audioFile.read(); 
//      dacWrite(DAC_OUT, sample); 
////      digitalWrite(TIMING_PIN, 0); 
//    } 
//    else 
//    {
//      repeatAlarm();    
//    }
//  }

  // display flag
  if(updateDisplayFlag)
  {
    digitalWrite(TIMING_PIN, 1);
    display.display(70);
    updateDisplayFlag = false;
    digitalWrite(TIMING_PIN, 0);
  }
  
  if(second_flag)
  {
    second++;
    display.clearDisplay();
    formatTime();
    display.print(the_time);
    second_flag = 0;
    alarmEnable = true; // temp workaround
  }
  if(second == 0 && alarmEnable)
  {
    alarmEnable = false; // temp workaround
    triggerAlarm("/hitsdifferent8.wav", 4);
  }
//  Serial.println("bottom of loop --------------");
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

void triggerAlarm(String alarmFile, int repeats) // wrapper/enum for alarm sounds
{
  Serial.println("Alarm was triggered!");
  Serial.print("repeatCount = ");
  Serial.println(repeatCount);
  repeatCount = repeats - 1;
  playWAV(alarmFile);
}

void playWAV(String fileName)
{
  // Open the WAV file
  audioFile = SPIFFS.open(fileName, "r"); 
  if (!audioFile) {
    Serial.println("Failed to open file");
    delay(3000);
    return;
  }

  // maybe move this all into if
  sampleTimer = timerBegin(1000000); // TODO try changing to 80
  if(audioFile.available())
  {
    timerAttachInterrupt(sampleTimer, &isr_play_sample);
    Serial.println("interrupt attached");
  }
  timerAlarm(sampleTimer, 62, true, 0); // 1/16000Hz = 62.5us
  audioFile.seek(44); // skip WAV header
}

void stopAlarm()
{
  timerEnd(sampleTimer);  
  audioFile.close();      
  repeatCount = 0;       
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
