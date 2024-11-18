volatile bool flag = 0;
hw_timer_t *timer = NULL;

void IRAM_ATTR alert()
{
  flag = 1;
//  Serial.println("ISR");
}

void setup() {
  Serial.begin(9600);
  pinMode(32, OUTPUT);
  timer = timerBegin(1000000);
  timerAttachInterrupt(timer, &alert);
  timerAlarm(timer, 1000000, true, 0);
}

void loop() 
{
  if(flag)
  {
//    Serial.println("Flag");
    digitalWrite(32, 1);
    delay(100);
    digitalWrite(32, 0);
    flag = 0;
  }
}
