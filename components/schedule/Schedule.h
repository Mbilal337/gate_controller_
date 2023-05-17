#ifndef GATE_SCHEDULE_H
#define GATE_SCHEDULE_H

#include "esp_sntp.h"
#include <time.h>
#include <sys/time.h>
#include "esp_log.h"
#include "esp_spiffs.h"
#include "stdio.h"
#include "string.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/timers.h"

#define TRUE 1
#define FALSE 0

#define ARRAY_SIZE(a) sizeof(a) / sizeof(a[0])

#define ONE_MINUTE (60 * 1000)
#define TIME_ONE SECOND 1000
#define TIME_FIVE_SECOND 1000 * 5
#define TIME_FIVE_MINUTE 5 * ONE_MINUTE
#define TIME_ONE_HOUR 60 * ONE_MINUTE

#define RELAYS_ATTACHED 2
#define CHECK_RELAY1 1
#define CHECK_RELAY2 2

extern bool isCheckforSchedule;
extern bool isStartupScheduleCheck;
extern bool isScheduleReadytoExecuteforRelay1;
extern bool isScheduleReadytoExecuteforRelay2;

typedef struct ParseSchedule
{
    char ActiontoPerform[15][5];
    char Days[15][264];
    char Buffer[15][500];
    int enabled[15];
    int hours[15];
    int index[15];
    int minutes[15];
    int repeat[15];
}ParseSchedule;

typedef struct ScheduleAnalyzer
{
    uint8_t Min[2][15];
    uint8_t Hours[2][15];
    uint8_t AlarmIndex[2];
    uint8_t Action[2][15];
}ScheduleAnalyzer;

typedef struct DayCheck
{
    int friday;
    int monday;
    int saturday;
    int sunday;
    int thursday;
    int tuesday;
    int wednesday;
}DayCheck;

extern struct ScheduleAnalyzer AnalyzeSchedule;

extern struct tm timeinfo;

void time_sync_notification_cb(struct timeval *tv);

void initialize_sntp(void);

void obtain_time(void);

struct Schedule_h Initialize_Scheduler();

struct Schedule_h ParseTimeString(char *TimeString, struct tm info);

bool checkforTodaySchedule(struct Schedule_h GetTime_t, uint8_t Relay);

void WriteSchedule(char *Schedule);

int MonthToNumber(char *month);

struct Schedule_h CheckDayinSchedule(char *Day);

struct Schedule_h GetCurrentTime();

int find_minimum(uint8_t a[], int n);

void moveAllZeroesAtArrayEnd(uint8_t arr[], int n);

int FindSimilarSchedulesSameHour(uint8_t a[], uint8_t b[], int n, int hour);

#endif