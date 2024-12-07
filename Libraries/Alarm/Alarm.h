#ifndef ALARM_H
#define ALARM_H

#include <SPIFFS.h>

#define ALARM_MINUTE(alarm_time) (uint8_t)((alarm_time >> 4) & 0xFF)
#define ALARM_HOUR(alarm_time) (uint8_t)((alarm_time >> 12) & 0xF)
#define ALARM_AM(alarm_time) (uint8_t)((alarm_time >> 3) & 1);

// alarm time format: hhhh mmmm mmmm a000
// e.g. 7:15am        0111 0000 1111 1000
extern uint16_t alarm_time;
extern File alarmFile;
extern uint8_t alarm_index = -1;
extern const char *alarmFiles[];

extern repeatCount;

bool checkAlarmTime();
void triggerAlarm(const char *fileName, int repeats);
void playWAV(const char *fileName);

#endif // ALARM_H