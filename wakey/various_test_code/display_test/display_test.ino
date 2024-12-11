#include <PxMatrix.h>

#ifdef ESP32

#define P_LAT 12
#define P_A 32
#define P_B 33
#define P_C 26
#define P_D 27
#define P_OE 22
hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

#endif

#ifdef ESP8266

#include <Ticker.h>
Ticker display_ticker;


#endif
// Pins for LED MATRIX

//PxMATRIX display(32,16,P_LAT, P_OE,P_A,P_B,P_C);
PxMATRIX display(32,32,P_LAT, P_OE,P_A,P_B,P_C,P_D);
//PxMATRIX display(64,64,P_LAT, P_OE,P_A,P_B,P_C,P_D,P_E);

#ifdef ESP8266
// ISR for display refresh
void display_updater()
{
  display.displayTestPattern(70);
  // display.displayTestPixel(70);
  //display.display(70);
}
#endif

#ifdef ESP32
void IRAM_ATTR display_updater(){
  // Increment the counter and set the time of ISR
  portENTER_CRITICAL_ISR(&timerMux);
  //isplay.display(70);
  display.displayTestPattern(70);
  portEXIT_CRITICAL_ISR(&timerMux);
}
#endif


uint16_t myCYAN = display.color565(0, 255, 255);
void setup() {
  // put your setup code here, to run once:
Serial.begin(9600);
  display.begin(16);
  display.flushDisplay();
  display.setTextColor(myCYAN);
  display.setCursor(2,0);
  display.print("Pixel");
  Serial.println("hello");

  #ifdef ESP8266
    display_ticker.attach(0.004, display_updater);
  #endif

  #ifdef ESP32
    timer = timerBegin(1000000); // 1MHz
    timerAttachInterrupt(timer, &display_updater);
    timerAlarm(timer, 4000, true, 0);
  #endif

  delay(1000);
}


void loop() {

 delay(100);

}
