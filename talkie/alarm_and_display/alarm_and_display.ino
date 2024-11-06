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

// Time Web API Setup
const char* ssid       = "DukeVisitor";
const char* password   = "";
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -18000; // Eastern Standard Time
const int   daylightOffset_sec = 3600; // observes daylight savings
char the_time[10];
volatile int minute;
volatile int hour;
volatile int second;

volatile bool stopFlag = false;
const int STOP_BTN = 32;
const int dacPin = 25;


// LED Matrix Display
#define P_LAT 22
#define P_A 19
#define P_B 23
#define P_C 18
#define P_D 5
#define P_E 15
#define P_OE 2
PxMATRIX display(32,32,P_LAT, P_OE,P_A,P_B,P_C,P_D);

// Time Setup
hw_timer_t * timer = NULL;
hw_timer_t *timer2 = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

volatile bool second_flag;

uint16_t randomColor()
{
  return random(65535);
}

void IRAM_ATTR isr_stop_button()
{
  stopFlag = true;
}

void IRAM_ATTR isr_second_passed()
{
  portENTER_CRITICAL_ISR(&timerMux);
  second_flag = 1;
//  second++;
  portEXIT_CRITICAL_ISR(&timerMux);
}

void IRAM_ATTR isr_display_updater()
{
  portENTER_CRITICAL_ISR(&timerMux);
  display.display(70);
  portEXIT_CRITICAL_ISR(&timerMux);
}

void formatTime();
void triggerAlarm();
void playWAV();

void setup() 
{
  Serial.begin(9600);

  // connect to WiFi
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.println(" CONNECTED");

  //init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
//  printLocalTime();

  // initialize random seed to disconnected analog pin input
  pinMode(32, INPUT);
  randomSeed(analogRead(32));
  
  display.begin(16); 
  display.flushDisplay();

  timer2 = timerBegin(1000000);
  timerAttachInterrupt(timer2, &isr_second_passed);
  timerAlarm(timer2, 1000000, true, 0);
  
  timer = timerBegin(1000000); // TODO try changing to 80
  timerAttachInterrupt(timer, &isr_display_updater);
  timerAlarm(timer, 4000, true, 0);

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
  second = timeinfo.tm_sec;

  formatTime();
  display.clearDisplay(); 
  display.print(the_time);
}

void loop() {
  if(second_flag)
  {
    second++;
    display.clearDisplay();
    formatTime();
    display.print(the_time);
    second_flag = 0;
  }
  if(second == 0)
  {
    triggerAlarm();
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

void triggerAlarm()
{
  Serial.println("Alarm was triggered!");
  while(!stopFlag)
  {
    playWAV("/ceilings16.wav");
  }
  stopFlag = false;
  Serial.println("Alarm is over.");
}

void playWAV(String fileName)
{
  File audioFile = SPIFFS.open(fileName, "r"); 
  if (!audioFile) {
    Serial.println("Failed to open file");
    delay(3000);
    return;
  }
  audioFile.seek(44); // skip WAV header
//  Serial.println(audioFile.available());
  delay(2000); // not sure if we need this

  // Enable button interrupt
  pinMode(STOP_BTN, INPUT);
  attachInterrupt(STOP_BTN, isr_stop_button, RISING); // might not want to do this each time
  
  // Read and play audio data
  while (audioFile.available() && stopFlag==0) 
  {
    int sample = audioFile.read();
    if (sample >= 0) {
      dacWrite(dacPin, sample);
    }
    delayMicroseconds(40); // will need to be fixed for sure
  }
  audioFile.close();
  detachInterrupt(STOP_BTN);
}
