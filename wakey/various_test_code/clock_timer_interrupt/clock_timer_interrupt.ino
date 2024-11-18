#include <Adafruit_GrayOLED.h>
#include <gfxfont.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SPITFT.h>
#include <Adafruit_SPITFT_Macros.h>
#include <PxMatrix.h>
#include <WiFi.h>

const char* ssid       = "DukeVisitor";
const char* password   = "";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -18000; // Eastern Standard Time
const int   daylightOffset_sec = 3600; // observes daylight savings

char the_time[10];
volatile int minute;
volatile int hour;
volatile int second;

#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF

uint16_t randomColor()
{
  return random(65535);
}

// Pins for LED MATRIX
#define P_LAT 22
#define P_A 19
#define P_B 23
#define P_C 18
#define P_D 5
#define P_E 15
#define P_OE 2
hw_timer_t * timer = NULL;
hw_timer_t *timer2 = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

PxMATRIX display(32,32,P_LAT, P_OE,P_A,P_B,P_C,P_D);

volatile bool flag;
void IRAM_ATTR alert()
{
  portENTER_CRITICAL_ISR(&timerMux); // Critical section for interrupt-safe updates
  flag = 1;
  second++;
  portEXIT_CRITICAL_ISR(&timerMux);
}

void IRAM_ATTR display_updater()
{
//  digitalWrite(32, 1);
  portENTER_CRITICAL_ISR(&timerMux);
  display.display(70);
  portEXIT_CRITICAL_ISR(&timerMux);
//  digitalWrite(32, 0);
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

void setup() 
{
  Serial.begin(9600);

  pinMode(32, OUTPUT);

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
  pinMode(34, INPUT);
  randomSeed(analogRead(34));
  
  display.begin(16); 
  display.flushDisplay();

  timer2 = timerBegin(1000000);
  timerAttachInterrupt(timer2, &alert);
  timerAlarm(timer2, 1000000, true, 0);
  
  timer = timerBegin(1000000); // TODO change to 80
  timerAttachInterrupt(timer, &display_updater);
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
  if(flag)
  {
    display.clearDisplay();
    formatTime();
    display.print(the_time);
//    digitalWrite(32, 1);
//    delay(10);
//    digitalWrite(32, 0);
    flag = 0;
  }
}
