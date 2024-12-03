#ifndef _RADAR_H__
#define _RADAR_H__

#include "Arduino.h"

// Data Frame Definitions
#define MESSAGE_HEAD 0x55       // Data frame header
#define ACTIVE_REPORT 0x04      // Proactive reporting

#define REPORT_RADAR 0x03       // Report radar information
#define REPORT_OTHER 0x05       // Report other information

#define HEARTBEAT 0x01          // Heartbeat Pack
#define ABNOEMAL 0x02           // Abnormal Reset
#define ENVIRONMENT 0x05        // Environment
#define BODYSIGN 0x06           // Physical parameters
#define CLOSE_AWAY 0x07         // Approach away

#define CA_BE 0x01              // Approach away head frame
#define CA_CLOSE 0x02           // Someone approaches
#define CA_AWAY 0x03            // Some people stay away
#define SOMEBODY_BE 0x01        // Motion state header frame
#define SOMEBODY_MOVE 0x01      // Somebody move
#define SOMEBODY_STOP 0x00      // Somebody stop
#define NOBODY 0x00             // No one here

#define SLEEP_INF 0x05          // Sleep radar data header frames
#define BREATH 0x01             // Breathing parameters
#define SCENARIO 0x03           // Scenario evaluation
#define SLEEP_TIME 0x04         // Duration parameters
#define SLEEP_QUALITY 0x05      // Sleep quality

#define BREATH_RATE 0x01        // Breathing rate
#define CHECK_SIGN 0x04         // Detection signal

#define BREATH_HOLD 0x01        // Breath-holding abnormalities
#define BREATH_NULL 0x02        // None
#define BREATH_NORMAL 0x03      // Normal breathing
#define BREATH_MOVE 0x04        // Movement abnormalities
#define BREATH_RAPID 0x05       // Acute respiratory abnormalities

#define CLOSE_AWAY_BED 0x07     // Getting in and out of bed judgment
#define SLEEP_STATE 0x08        // Sleep state judgment

#define AWAY_BED 0x00           // Leaving the bed
#define CLOSE_BED 0x01          // Get into bed

#define AWAKE 0x00              // Sleep state: Awake
#define LIGHT_SLEEP 0x01        // Sleep state: Light sleep
#define DEEP_SLEEP 0x02         // Sleep state: Deep sleep
#define SLEEP_NULL 0x03         // No sleep state

#define AWAKE_TIME 0x01         // Awake time
#define LIGHT_SLEEP_TIME 0x02   // Light sleep time
#define DEEP_SLEEP_TIME 0x03    // Deep sleep time

#define SLEEP_SCORE 0x01        // Sleep quality score


class SleepBreathingRadar {
private:
    HardwareSerial* radarSerial; // Pointer to custom HardwareSerial instance

public:
    const byte MsgLen = 12;
    byte dataLen = 12;
    byte Msg[12];
    boolean newData = false;
    int CURRENT_STATE = -1; // Track the current sleep state

    void SerialInit(HardwareSerial* serial); // Initialize with a custom serial instance
    void recvRadarBytes();                   // Receive and process data
    void Bodysign_judgment(byte inf[], float Move_min, float Move_max);
    void Situation_judgment(byte inf[]);
    void Sleep_inf(byte inf[]);
    void SleepTimeCalculate(unsigned char inf1, unsigned char inf2, unsigned char inf3, unsigned char inf4);
    void ShowData(byte inf[]);
    unsigned short int us_CalculateCrc16(unsigned char* lpuc_Frame, unsigned short int lus_Len);
};

#endif
