#include <Adafruit_GFX.h>
#include <Adafruit_GrayOLED.h>
#include <gfxfont.h>
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

#define TIMING_PIN 27
#define STOP_BTN 21

PxMATRIX display(32, 32, P_LAT, P_OE, P_A, P_B, P_C, P_D);

// Time Setup
hw_timer_t *displayTimer = NULL;
hw_timer_t *clockTimer = NULL;
hw_timer_t *sampleTimer = NULL;

#define BUFFER_SIZE 1024

File audioFile; 
char *playing_buf;
char *filling_buf;
char *tmp;
int playing_buf_size = 0;
int filling_buf_size = 0;
int playing_idx = 0;
int sample;
int repeatCount;

// alarm time format: hhhh mmmm mmmm 000a
// e.g. 7:15am        0111 0000 1111 0001
//uint16_t alarmTime = 0x; 
uint16_t ALARM_MINUTES = 0x0FF0;
uint16_t ALARM_HOURS = 0xF000;
uint16_t ALARM_AM = 0x0001;

void formatTime();
void playWAV(String fileName);
void fillBuffer();
void switchBuffers();
void triggerAlarm(String fileName, int repeats);
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

  sample = 0;
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

  displayTimer = timerBegin(1000000); 
  timerAttachInterrupt(displayTimer, &isr_display_updater);
  timerAlarm(displayTimer, 5000, true, 0); 

  clockTimer = timerBegin(1000000);
  timerAttachInterrupt(clockTimer, &isr_second_passed);
  timerAlarm(clockTimer, 1000000, true, 0);

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

  // Initialize SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS mount failed");
    delay(3000);
    return;
  }

  playing_buf = (char *)malloc(BUFFER_SIZE);
  filling_buf = (char *)malloc(BUFFER_SIZE);

  triggerAlarm("/hitsdifferent8.wav", 3);
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
    secondFlag = false;
    // checkForAlarm();
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
}

// void checkForAlarm()
// {
//   Serial.println("Checking for alarm...");
//   Serial.print("minute: ");
//   Serial.println((uint8_t)minute);
//   Serial.print("(alarmTime && ALARM_MINUTE) >> 4: ");
//   Serial.println((uint8_t)(alarmTime && ALARM_MINUTES) >> 4);
// //  if((uint8_t)minute == (uint8_t)(alarmTime && ALARM_MINUTE) >> 4));
// }

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

void triggerAlarm(String fileName, int repeats)
{
  Serial.println("Alarm was triggered!");
  repeatCount = repeats - 1;
  playWAV(fileName);
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

  if(audioFile.available())
  {
    fillBuffer();
    switchBuffers();
    sampleTimer = timerBegin(1000000); 
    timerAttachInterrupt(sampleTimer, &isr_play_sample);
    timerAlarm(sampleTimer, 62, true, 0); // 1/16000Hz = 62.5us
    Serial.println("WAV file playing");
  }
  
  audioFile.seek(44); // skip WAV header
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
