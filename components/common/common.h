#ifndef _COMMON_H
#define _COMMON_H

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#define HARD_V_1 1
#define HARD_V_2 2
#define DEBUG_ON
#define LED_DUTY_DEFAULT 0
#define LED_DUTY_GREEN 1
#define LED_DUTY_BLUE 2
#define LED_DUTY_RED 3

///variables needed for relay task 
extern bool relay1_status;
extern bool relay2_status;
extern bool reed1_status_changed;
extern bool reed2_status_changed;
extern bool device_settings_updated;
extern bool relay1_open_state ;
extern bool relay2_open_state ;


bool reedStatus1 ;
bool reedStatus2 ;
extern int autoClose1 ;
extern int autoClose2 ;

//other variables 

extern bool now_read_flash;

//common variables 

// common
extern int tempAlert ;
extern bool alertOnClose ;
extern bool alertOnOpen ;
extern bool nightAlert ;
extern char *region ;
// int wifi_connected = 1 ;
// int AP_mode = 0 ; 
// int gsm_connected = 0 ; 



extern int duty_change_last_call;
extern char mac_value[30];
extern char *GIOT_DEVICE_ID;

extern char *OFFSET;

typedef struct Schedule_h
{
    char Day[3];
    int Date;
    int Year;
    int Hour;
    int Minute;
    int Month;
    char checkDay[10];
}GateSchedule;

extern int OFFSET_H; //5 is default 
extern int OFFSET_M;  //0 min as default 
extern int  OFFSET_S ; //0 sec as default 


#endif