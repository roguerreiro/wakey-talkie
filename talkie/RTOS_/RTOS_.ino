#define TIMING_PIN 27
 
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
volatile bool displayFlag = false;
volatile bool secondFlag = false;
volatile bool sampleFlag = false;

int sample;

File audioFile; 

// LED Matrix Display
#define P_LAT 22
#define P_A 19
#define P_B 23
#define P_C 18
#define P_D 5
#define P_OE 15
#define DAC_OUT 25

PxMATRIX display(32,32,P_LAT, P_OE,P_A,P_B,P_C,P_D);

#define RANDOM_PIN 34

// Time Setup
hw_timer_t *displayTimer = NULL;
hw_timer_t *clockTimer = NULL;
hw_timer_t *sampleTimer = NULL;

portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR isr_play_sample()
{
  sampleFlag = true;
}

uint16_t randomColor()
{
  return random(65535);
}

//  dacWrite(DAC_OUT, sample);
//  sampleFlag = 1;


void formatTime();
void playWAV(String fileName);

void audioTask(void *parameter)
{
  while (audioFile.available()) {
    if (sampleFlag) {
      sampleFlag = false;  // Clear the flag

      digitalWrite(TIMING_PIN, 1);
      // Read one byte (sample) from the audio file and write to DAC
      int sample = audioFile.read();
      dacWrite(DAC_OUT, sample);
      digitalWrite(TIMING_PIN, 0);
      
      // You can add additional processing here if needed, such as buffering
    }
 }

  // Close the file when done
  audioFile.close();
  vTaskDelete(NULL);  // End the task
}

void displayTask(void *parameter) 
{
    TickType_t lastWakeTime = xTaskGetTickCount(); // Capture current time in ticks
    const TickType_t interval = 5 / portTICK_PERIOD_MS; // 5 ms interval

    while (1) {
//      digitalWrite(TIMING_PIN, 1);
        display.display(10);
//        digitalWrite(TIMING_PIN, 0);
        // Delay until the next 5 ms period
        vTaskDelayUntil(&lastWakeTime, interval);
    }
}

void clockTask(void *parameter) 
{
    const TickType_t xDelay = 1000 / portTICK_PERIOD_MS; // 1000 ms = 1 second
    while (1) {
        // Clock task work
        
      second++;
      display.clearDisplay();
      formatTime();
      display.print(the_time);
        // Delay for 1 second
        vTaskDelay(xDelay); // This delays the task for 1 second
    }
}


void setup() 
{
  Serial.begin(115200);

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

//  displayTimer = timerBegin(1000000); 
//  timerAttachInterrupt(displayTimer, &isr_display_updater);
//  timerAlarm(displayTimer, 5000, true, 0); 
//
  sampleTimer = timerBegin(1000000);
  timerAttachInterrupt(sampleTimer, &isr_play_sample);
  timerAlarm(sampleTimer, 62, true, 0);

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

//  xTaskCreatePinnedToCore(audioTask, "AudioTask", 2048, NULL, 3, NULL, 1);
  xTaskCreatePinnedToCore(displayTask, "DisplayTask", 2048, NULL, 2, NULL, 1);
  xTaskCreatePinnedToCore(clockTask, "TimekeepingTask", 1024, NULL, 1, NULL, 1);


  playWAV("/hitsdifferent8.wav");

}

void loop() 
{
//  if(sampleFlag)
//  {
////    digitalWrite(TIMING_PIN, 1);
//    sample = audioFile.read(); 
////    dacWrite(DAC_OUT, sample);
//    sampleFlag = false;
////    digitalWrite(TIMING_PIN, 0);
//  }
//  if(displayFlag)
//  {
//    display.display(10);
//    displayFlag = false;
//  }
//  if(secondFlag)
//  {
////    digitalWrite(TIMING_PIN, 1);
//    second++;
//    display.clearDisplay();
//    formatTime();
//    display.print(the_time);
//    secondFlag = false;
////    digitalWrite(TIMING_PIN, 0);
//  }
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
  timerAlarm(sampleTimer, 124, true, 0); // 1/16000Hz = 62.5us
  audioFile.seek(44); // skip WAV header

  // start task
    xTaskCreatePinnedToCore(audioTask, "AudioTask", 2048, NULL, 3, NULL, 1);
}
