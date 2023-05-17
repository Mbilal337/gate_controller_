#include "Schedule.h"
#include "cJSON.h"
#include "cJSON_Utils.h"
#include "string.h"
#include "common.h"
#include "nvs.h"

bool isCheckforSchedule = FALSE;
bool isScheduleReadytoExecuteforRelay1 = FALSE;
bool isScheduleReadytoExecuteforRelay2 = FALSE;
bool isStartupScheduleCheck = FALSE;

struct ScheduleAnalyzer AnalyzeSchedule = {0};

struct tm timeinfo = {0};

TimerHandle_t xTimerRelay1;
TimerHandle_t xTimerRelay2;
const char *TAG_SCHEDULE = "[Schedule Handler]:";

void configure_offset(char *offset) // function to configure hour, min and sec from string getting at registration
{
    const char deli[] = ":";
    char *token;
    token = strtok(offset, deli);

    for (int i = 0; i < 3; i++)
    {
        if (i == 0)
        {
            OFFSET_H = atoi(token);
            printf("hour %d \n ", atoi(token)); // printf("%s", str); will print the same result
        }
        else if (i == 1)
        {
            OFFSET_M = atoi(token);
            printf(" min %d\n", OFFSET_M); // printing each token
        }
        else
        {
            OFFSET_S = atoi(token);
            printf(" sec %d\n", OFFSET_S); // printing each token
        }
        token = strtok(NULL, ":");
    }
}

void vTimerCallback(TimerHandle_t xTimerRelay1)
{
    isCheckforSchedule = TRUE;
    ESP_LOGI(TAG_SCHEDULE, "Checking for Schedule");
}

void time_sync_notification_cb(struct timeval *tv)
{
    ESP_LOGI(TAG_SCHEDULE, "Notification of a time synchronization event");
}

void initialize_sntp(void)
{
    ESP_LOGI(TAG_SCHEDULE, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_setservername(1, "europe.pool.ntp.org");
    sntp_setservername(2, "uk.pool.ntp.org ");
    sntp_setservername(3, "us.pool.ntp.org");
    sntp_setservername(4, "time1.google.com");

    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
#ifdef CONFIG_SNTP_TIME_SYNC_METHOD_SMOOTH
    sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);
#endif
    sntp_init();
}

void obtain_time(void)
{
    /**
     * NTP server address could be aquired via DHCP,
     * see LWIP_DHCP_GET_NTP_SRV menuconfig option
     */
#ifdef LWIP_DHCP_GET_NTP_SRV
    sntp_servermode_dhcp(1);
#endif

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    char strftime_buf[64];

    // Set timezone to Eastern Standard Time and print local time

    esp_err_t err;
    char *key;
    key = "offset";
    err = nvs_read(key, &OFFSET);
    if (err != ESP_OK)
    {
        printf("\n\n\nUnable to get OFFSET from flash \n\n\n");
        OFFSET = "05:00:00";
    }
    configure_offset(OFFSET);
    int hr = -1 * (OFFSET_H);
    char hour[7];
    if (hr > 0)
    {
        sprintf(hour, "+%d:%d:%d", hr, OFFSET_M, OFFSET_S);
    }
    else
    {
        sprintf(hour,"%d:%d:%d", hr, OFFSET_M, OFFSET_S);
    }
    char tz[9] = "UTC";
    strcat(tz, hour);
    printf("timezone is : %s \n ", tz);
    setenv("TZ", tz, 1);
    tzset();

    // wait for time to be set
    time_t now = 0;

    int retry = 0;
    const int retry_count = 5;

    while (timeinfo.tm_year < (2016 - 1900))
    {
        ESP_LOGI(TAG_SCHEDULE, "Waiting for system time to be set...");
        vTaskDelay(2000 / portTICK_PERIOD_MS);

        time(&now);
        localtime_r(&now, &timeinfo);
        // strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    }

    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count)
    {
        ESP_LOGI(TAG_SCHEDULE, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    time(&now);
    localtime_r(&now, &timeinfo);
    // ESP_LOGI(TAG_SCHEDULE, "Time is set...");
}

struct Schedule_h Initialize_Scheduler()
{
    struct Schedule_h get_time;
    char strftime_buf[64];
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    // Is time set? If not, tm_year will be (1970 - 1900).
    if (timeinfo.tm_year < (2016 - 1900))
    {
        ESP_LOGI(TAG_SCHEDULE, "Time is not set yet. Connecting to WiFi and getting time over NTP.");
        obtain_time();
        // update 'now' variable with current time
        time(&now);
    }

    if (sntp_get_sync_mode() == SNTP_SYNC_MODE_SMOOTH)
    {
        struct timeval outdelta;
        while (sntp_get_sync_status() == SNTP_SYNC_STATUS_IN_PROGRESS)
        {
            adjtime(NULL, &outdelta);
            ESP_LOGI(TAG_SCHEDULE, "Waiting for adjusting time ... outdelta = %li sec: %li ms: %li us",
                     (long)outdelta.tv_sec,
                     outdelta.tv_usec / 1000,
                     outdelta.tv_usec % 1000);
            vTaskDelay(2000 / portTICK_PERIOD_MS);
        }
    }

    time(&now);
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI(TAG_SCHEDULE, "The current date/time in Pakistan is: %s", strftime_buf);
    struct Schedule_h ParsedStruct = ParseTimeString(strftime_buf, timeinfo);
    strcpy(get_time.Day, ParsedStruct.Day);
    get_time.Date = ParsedStruct.Date;
    get_time.Hour = ParsedStruct.Hour;
    get_time.Minute = ParsedStruct.Minute;
    get_time.Year = ParsedStruct.Year;
    get_time.Month = ParsedStruct.Month;

    // ESP_LOGI(TAG, "The current date/time in Pakistan is: %d",get_time->Year);
    ESP_LOGI(TAG_SCHEDULE, "Initializing SPIFFS");

    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true};

    // Use settings defined above to initialize and mount SPIFFS filesystem.
    // Note: esp_vfs_spiffs_register is an all-in-one convenience function.
    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK)
    {
        if (ret == ESP_FAIL)
        {
            ESP_LOGE(TAG_SCHEDULE, "Failed to mount or format filesystem");
        }
        else if (ret == ESP_ERR_NOT_FOUND)
        {
            ESP_LOGE(TAG_SCHEDULE, "Failed to find SPIFFS partition");
        }
        else
        {
            ESP_LOGE(TAG_SCHEDULE, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return;
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(conf.partition_label, &total, &used);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG_SCHEDULE, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    }
    else
    {
        ESP_LOGI(TAG_SCHEDULE, "Partition size: total: %d, used: %d", total, used);
    }

    return get_time;
}

bool checkforTodaySchedule(struct Schedule_h GetTime_t, uint8_t Relay)
{
    ESP_LOGI(TAG_SCHEDULE, "Checking Schedule for %s", GetTime_t.Day);
    struct stat st;
    char RelayIdentifier[17];
    char line[4096] = {0};
    if (Relay == 0)
    {
        Relay = 1; // Only to avoid compilation error
    }
    memset(RelayIdentifier, 0, sizeof(RelayIdentifier));
    sprintf(RelayIdentifier, "/spiffs/R%d.txt", Relay);

    if (stat(RelayIdentifier, &st) == 0)
    {
        ESP_LOGI(TAG_SCHEDULE, "Reading file %s", RelayIdentifier);
        FILE *f = fopen(RelayIdentifier, "r");
        if (f == NULL)
        {
            ESP_LOGE(TAG_SCHEDULE, "Failed to open file for reading");
            return;
        }

        fgets(line, sizeof(line), f);
        fclose(f);

        // strip newline
        char *pos = strchr(line, '\n');
        if (pos)
        {
            *pos = '\0';
        }
        ESP_LOGI(TAG_SCHEDULE, "Read from file: '%s': %s", RelayIdentifier, line);

        cJSON *isScheduleEnabled = NULL;
        cJSON *Monitor_json = cJSON_Parse(line);
        cJSON *ArrayCheck = NULL;
        if (Monitor_json != NULL)
        {
            ESP_LOGI(TAG_SCHEDULE, "Parsing JSON");
            cJSON *GetScheduleArray = cJSON_GetObjectItem(Monitor_json, "schedules");
            int JsonArraySize = cJSON_GetArraySize(GetScheduleArray);
            ESP_LOGW(TAG_SCHEDULE, "JSON Array Size %d", JsonArraySize);
            int index = 0;

            cJSON_ArrayForEach(ArrayCheck, GetScheduleArray)
            {
                isScheduleEnabled = cJSON_GetObjectItem(ArrayCheck, "enabled");
                if (isScheduleEnabled != NULL)
                {
                    ESP_LOGI(TAG_SCHEDULE, "enabled: %d", isScheduleEnabled->valueint);
                    if (isScheduleEnabled->valueint == 1)
                    {
                        ESP_LOGI(TAG_SCHEDULE, "Schedule is Enabled");
                        struct Schedule_h DayToday;

                        cJSON *targetDay = cJSON_GetObjectItem(ArrayCheck, "days");
                        DayToday = CheckDayinSchedule(GetTime_t.Day);
                        cJSON *nestedDay = cJSON_GetObjectItem(targetDay, DayToday.checkDay);
                        ESP_LOGI(TAG_SCHEDULE, "Checking for Day %s", DayToday.checkDay);

                        if (nestedDay != NULL && nestedDay->valueint == 1)
                        {
                            ESP_LOGI(TAG_SCHEDULE, "Schedule for %s = %d", DayToday.checkDay, nestedDay->valueint);
                            cJSON *TargetHour = cJSON_GetObjectItem(ArrayCheck, "hours");
                            cJSON *TargetMinute = cJSON_GetObjectItem(ArrayCheck, "minutes");
                            cJSON *ActionRelay = cJSON_GetObjectItem(ArrayCheck, "actionToPerform");
                            if (strcmp(ActionRelay, "OPEN") == 0)
                            {
                                AnalyzeSchedule.Action[Relay][index] = 1;
                            }
                            else
                            {
                                AnalyzeSchedule.Action[Relay][index] = 1;
                            }
                            ESP_LOGI(TAG_SCHEDULE, "Schedule Found for today at %d:%d", TargetHour->valueint, TargetMinute->valueint);
                            AnalyzeSchedule.Hours[Relay][index] = TargetHour->valueint;
                            AnalyzeSchedule.Min[Relay][index] = TargetMinute->valueint;
                            ESP_LOGW(TAG_SCHEDULE, "H: %d | M: %d | index %d", AnalyzeSchedule.Hours[Relay][index], AnalyzeSchedule.Min[Relay][index], index);
                        }
                    }
                    else
                    {
                        ESP_LOGW(TAG_SCHEDULE, "Schedule not enabled");
                    }
                }
                else
                {
                    ESP_LOGI(TAG_SCHEDULE, "enabled: NOT");
                }
                index++;
            }
            for (uint8_t i = 0; i < JsonArraySize; i++)
            {
                int differenceHour = 0;
                moveAllZeroesAtArrayEnd(AnalyzeSchedule.Hours[Relay], 15);
                moveAllZeroesAtArrayEnd(AnalyzeSchedule.Min[Relay], 15);
                for (uint8_t i = 0; i < JsonArraySize; i++)
                {
                    printf("%d\n", AnalyzeSchedule.Hours[Relay][i]);
                }
                int location = find_minimum(AnalyzeSchedule.Hours[Relay], index);
                int minlocation = FindSimilarSchedulesSameHour(AnalyzeSchedule.Hours[Relay], AnalyzeSchedule.Min[Relay], index, AnalyzeSchedule.Hours[Relay][location]);

                AnalyzeSchedule.AlarmIndex[Relay] = location;
                ESP_LOGI(TAG_SCHEDULE, "Minimum Hour at %d | index %d", location, index);
                differenceHour = AnalyzeSchedule.Hours[Relay][location] - GetTime_t.Hour;
                int differenceMinute = 0;
                ESP_LOGI(TAG_SCHEDULE, "Hours remaining for Schedule %d | scheduled %d | current %d", differenceHour, AnalyzeSchedule.Hours[Relay][location], GetTime_t.Hour);

                if (differenceHour > 1)
                {
                    if (Relay == CHECK_RELAY1)
                    {
                        xTimerRelay1 = xTimerCreate("Timer", pdMS_TO_TICKS(TIME_ONE_HOUR), pdTRUE, 0, vTimerCallback);
                        if (xTimerRelay1 != NULL)
                        {
                            ESP_LOGI(TAG_SCHEDULE, "Start Timer for 1 hour check");
                            xTimerStart(xTimerRelay1, 0);
                            return TRUE;
                        }
                    }
                    else
                    {
                        xTimerRelay2 = xTimerCreate("Timer", pdMS_TO_TICKS(TIME_ONE_HOUR), pdTRUE, 0, vTimerCallback);
                        if (xTimerRelay2 != NULL)
                        {
                            ESP_LOGI(TAG_SCHEDULE, "Start Timer for 1 hour check");
                            xTimerStart(xTimerRelay2, 0);
                            return TRUE;
                        }
                    }
                }
                else if (differenceHour == 0)
                {
                    if (minlocation != FALSE)
                    {
                        differenceMinute = AnalyzeSchedule.Min[Relay][minlocation] - GetTime_t.Minute;
                    }
                    else
                    {
                        differenceMinute = AnalyzeSchedule.Min[Relay][location] - GetTime_t.Minute;
                    }

                    ESP_LOGI(TAG_SCHEDULE, "Minutes remaining for Schedule %d for Relay %d", differenceMinute, Relay);
                    if (differenceMinute > 5)
                    {
                        if (Relay == CHECK_RELAY1)
                        {
                            if (xTimerRelay1 == NULL)
                            {
                                xTimerRelay1 = xTimerCreate("Timer", pdMS_TO_TICKS(TIME_FIVE_MINUTE), pdTRUE, 0, vTimerCallback);
                                if (xTimerRelay1 != NULL)
                                {
                                    ESP_LOGI(TAG_SCHEDULE, "Start Timer for 5 Minute check");
                                    xTimerStart(xTimerRelay1, 0);
                                    return TRUE;
                                }
                            }
                            else
                            {
                                xTimerStop(xTimerRelay1, 0);
                                xTimerChangePeriod(xTimerRelay1, pdMS_TO_TICKS(TIME_FIVE_MINUTE), 1);
                                if (xTimerRelay1 != NULL)
                                {
                                    ESP_LOGI(TAG_SCHEDULE, "Start Timer for 5 Minute check");
                                    xTimerStart(xTimerRelay1, 0);
                                    return TRUE;
                                }
                            }
                        }
                        else
                        {
                            if (xTimerRelay2 == NULL)
                            {
                                xTimerRelay2 = xTimerCreate("Timer", pdMS_TO_TICKS(TIME_FIVE_MINUTE), pdTRUE, 0, vTimerCallback);
                                if (xTimerRelay2 != NULL)
                                {
                                    ESP_LOGI(TAG_SCHEDULE, "Start Timer for 5 Minute check");
                                    xTimerStart(xTimerRelay2, 0);
                                    return TRUE;
                                }
                            }
                            else
                            {
                                xTimerStop(xTimerRelay2, 0);
                                xTimerChangePeriod(xTimerRelay1, pdMS_TO_TICKS(TIME_FIVE_MINUTE), 1);
                                if (xTimerRelay2 != NULL)
                                {
                                    ESP_LOGI(TAG_SCHEDULE, "Start Timer for 5 Minute check");
                                    xTimerStart(xTimerRelay2, 0);
                                    return TRUE;
                                }
                            }
                        }
                    }
                    else if ((differenceMinute > 0) && differenceMinute <= 5)
                    {
                        if (Relay == CHECK_RELAY1)
                        {
                            if (xTimerRelay1 == NULL)
                            {
                                xTimerRelay1 = xTimerCreate("Timer", pdMS_TO_TICKS(ONE_MINUTE), pdTRUE, 0, vTimerCallback);
                                if (xTimerRelay1 != NULL)
                                {
                                    ESP_LOGI(TAG_SCHEDULE, "Start Timer for 5 Minute check");
                                    xTimerStart(xTimerRelay1, 0);
                                    return TRUE;
                                }
                            }
                            else
                            {
                                xTimerStop(xTimerRelay1, 0);
                                xTimerChangePeriod(xTimerRelay1, pdMS_TO_TICKS(ONE_MINUTE), 1);
                                if (xTimerRelay1 != NULL)
                                {
                                    ESP_LOGI(TAG_SCHEDULE, "Start Timer for 5 Minute check");
                                    xTimerStart(xTimerRelay1, 0);
                                    return TRUE;
                                }
                            }
                        }
                        else
                        {
                            if (xTimerRelay2 == NULL)
                            {
                                xTimerRelay2 = xTimerCreate("Timer", pdMS_TO_TICKS(ONE_MINUTE), pdTRUE, 0, vTimerCallback);
                                if (xTimerRelay2 != NULL)
                                {
                                    ESP_LOGI(TAG_SCHEDULE, "Start Timer for 5 Minute check");
                                    xTimerStart(xTimerRelay2, 0);
                                    return TRUE;
                                }
                            }
                            else
                            {
                                xTimerStop(xTimerRelay2, 0);
                                xTimerChangePeriod(xTimerRelay1, pdMS_TO_TICKS(ONE_MINUTE), 1);
                                if (xTimerRelay2 != NULL)
                                {
                                    ESP_LOGI(TAG_SCHEDULE, "Start Timer for 5 Minute check");
                                    xTimerStart(xTimerRelay2, 0);
                                    return TRUE;
                                }
                            }
                        }
                    }
                    else if (differenceMinute == 0)
                    {
                        if (Relay == CHECK_RELAY1)
                        {
                            if (xTimerRelay1 == NULL)
                            {
                                isCheckforSchedule = TRUE;
                            }
                            ESP_LOGI(TAG_SCHEDULE, "Executing Scheduled Task for Relay");
                            isScheduleReadytoExecuteforRelay1 = TRUE;
                        }
                        else
                        {
                            if (xTimerRelay2 == NULL)
                            {
                                isCheckforSchedule = TRUE;
                            }
                            ESP_LOGI(TAG_SCHEDULE, "Executing Scheduled Task for Relay");
                            isScheduleReadytoExecuteforRelay2 = TRUE;
                            cJSON *TargetRepeat = cJSON_GetObjectItem(Monitor_json, "repeat");
                        }
                    }
                    else
                    {
                        cJSON *TargetRepeat = cJSON_GetObjectItem(Monitor_json, "repeat");
                        if (cJSON_IsTrue(TargetRepeat))
                        {
                            ESP_LOGW(TAG_SCHEDULE, "Time for today's Schedule is Already Elapsed");
                            if (xTimerRelay1 == NULL)
                            {
                                xTimerRelay1 = xTimerCreate("Timer", pdMS_TO_TICKS(TIME_ONE_HOUR), pdTRUE, 0, vTimerCallback);
                                if (xTimerRelay1 != NULL)
                                {
                                    ESP_LOGI(TAG_SCHEDULE, "Start Timer for 1 hOUR check");
                                    xTimerStart(xTimerRelay1, 0);
                                    return TRUE;
                                }
                            }
                            else
                            {
                                xTimerStop(xTimerRelay1, 0);
                                xTimerChangePeriod(xTimerRelay1, pdMS_TO_TICKS(TIME_ONE_HOUR), 1);
                                if (xTimerRelay1 != NULL)
                                {
                                    ESP_LOGI(TAG_SCHEDULE, "Start Timer for 1 Hour check");
                                    xTimerStart(xTimerRelay1, 0);
                                    return TRUE;
                                }
                            }
                        }
                    }
                    return TRUE;
                }

                else
                {
                    // return FALSE;
                }
                AnalyzeSchedule.Hours[Relay][location] = 0;
                AnalyzeSchedule.Min[Relay][location] = 0;
            }
        }
        else
        {
            ESP_LOGI(TAG_SCHEDULE, "Schedule Not found");
        }
    }
    return FALSE;
}

void WriteSchedule(char *Schedule)
{
    struct ParseSchedule ScheduleParse;
    // ESP_LOGI(TAG_SCHEDULE, "%s",Schedule);
    cJSON *Monitor_json = cJSON_Parse(Schedule);
    if (Monitor_json != NULL)
    {
        char TempBuffer[2048];
        ESP_LOGW(TAG_SCHEDULE, "[VALUE] Parsing Json");
        for (uint8_t i = 1; i < 3; i++)
        {
            char RelayIdentifier[7];
            sprintf(RelayIdentifier, "Relay%d", i);
            char ScheduleName[16];
            sprintf(ScheduleName, "/spiffs/R%d.txt", i);
            cJSON *RelaySchedule = cJSON_GetObjectItem(Monitor_json, RelayIdentifier);
            if (cJSON_IsObject(RelaySchedule))
            {
                ESP_LOGI(TAG_SCHEDULE, "%s is Object", RelayIdentifier);
                cJSON *ExtInput = cJSON_GetObjectItem(RelaySchedule, "ExtInput");
                cJSON *Name = cJSON_GetObjectItem(RelaySchedule, "Name");
                cJSON *OutTime = cJSON_GetObjectItem(RelaySchedule, "OutTime");
                cJSON *autoClose = cJSON_GetObjectItem(RelaySchedule, "autoClose");
                cJSON *reedSwitch = cJSON_GetObjectItem(RelaySchedule, "reedSwitch");
                cJSON *GetScheduleArray = cJSON_GetObjectItem(RelaySchedule, "schedules");
                cJSON *ScheduleINDEX = NULL;
                int extInp = ExtInput->valueint;
                char NameStr[20];
                strcpy(NameStr, Name->valuestring);
                int Otime = OutTime->valueint;
                int aClose = autoClose->valueint;
                int rSwitch = reedSwitch->valueint;
                if (cJSON_IsArray(GetScheduleArray))
                {
                    sprintf(TempBuffer, "{\"ExtInput\":%d,\"Name\":\"%s\",\"OutTime\":%d,\"autoClose\":%d,\"reedSwitch\":%d,\"schedules\":[", extInp, NameStr, Otime, aClose, rSwitch);
                    ESP_LOGI(TAG_SCHEDULE, "%s | \n LENGTH %d", TempBuffer, strlen(TempBuffer));
                    uint8_t Array_Size = cJSON_GetArraySize(GetScheduleArray);
                    int BufferIndex = strlen(TempBuffer);
                    uint8_t index = 0;
                    ESP_LOGI(TAG_SCHEDULE, "Array Size is %d", Array_Size);
                    cJSON_ArrayForEach(ScheduleINDEX, GetScheduleArray)
                    {
                        char ScheduletempBuffer[160];
                        cJSON *Action_to_perform = cJSON_GetObjectItem(ScheduleINDEX, "actionToPerform");
                        cJSON *enabled_t = cJSON_GetObjectItem(ScheduleINDEX, "enabled");
                        cJSON *hours_t = cJSON_GetObjectItem(ScheduleINDEX, "hours");
                        cJSON *index_t = cJSON_GetObjectItem(ScheduleINDEX, "index");
                        cJSON *minutes_t = cJSON_GetObjectItem(ScheduleINDEX, "minutes");
                        cJSON *repeat_t = cJSON_GetObjectItem(ScheduleINDEX, "repeat");
                        cJSON *Days_t = cJSON_GetObjectItem(ScheduleINDEX, "days");
                        strcpy(ScheduleParse.ActiontoPerform[index], Action_to_perform->valuestring);
                        ScheduleParse.enabled[index] = enabled_t->valueint;
                        ScheduleParse.hours[index] = hours_t->valueint;
                        ScheduleParse.index[index] = index_t->valueint;
                        ScheduleParse.minutes[index] = minutes_t->valueint;
                        ScheduleParse.repeat[index] = repeat_t->valueint;
                        sprintf(ScheduletempBuffer, "{\"actionToPerform\":\"%s\",\"enabled\":%d,\"hours\":%d,\"index\":%d,\"minutes\":%d,\"repeat\":%d,", ScheduleParse.ActiontoPerform[index], ScheduleParse.enabled[index], ScheduleParse.hours[index], ScheduleParse.index[index], ScheduleParse.minutes[index], ScheduleParse.repeat[index]);
                        // ESP_LOGE(TAG_SCHEDULE,"%s",ScheduletempBuffer);
                        strcat(TempBuffer, ScheduletempBuffer);
                        // ESP_LOGW(TAG_SCHEDULE,"[Append 431]:  %d bytes AFTER | %s",strlen(TempBuffer),TempBuffer);
                        if (cJSON_IsObject(Days_t))
                        {
                            BufferIndex = strlen(TempBuffer);
                            struct DayCheck CheckDays;
                            cJSON *FRI = cJSON_GetObjectItem(Days_t, "friday");
                            cJSON *MON = cJSON_GetObjectItem(Days_t, "monday");
                            cJSON *SAT = cJSON_GetObjectItem(Days_t, "saturday");
                            cJSON *SUN = cJSON_GetObjectItem(Days_t, "sunday");
                            cJSON *THUR = cJSON_GetObjectItem(Days_t, "thursday");
                            cJSON *TUES = cJSON_GetObjectItem(Days_t, "tuesday");
                            cJSON *WED = cJSON_GetObjectItem(Days_t, "wednesday");
                            CheckDays.friday = FRI->valueint;
                            CheckDays.monday = MON->valueint;
                            CheckDays.saturday = SAT->valueint;
                            CheckDays.sunday = SUN->valueint;
                            CheckDays.thursday = THUR->valueint;
                            CheckDays.tuesday = TUES->valueint;
                            CheckDays.wednesday = WED->valueint;
                            if (index + 1 != Array_Size)
                            {
                                sprintf(ScheduleParse.Days[index], "\"days\":{\"friday\":%d,\"monday\":%d,\"saturday\":%d,\"sunday\":%d,\"thursday\":%d,\"tuesday\":%d,\"wednesday\":%d}},", CheckDays.friday, CheckDays.monday, CheckDays.saturday, CheckDays.sunday, CheckDays.thursday, CheckDays.tuesday, CheckDays.wednesday);
                            }
                            else
                            {
                                sprintf(ScheduleParse.Days[index], "\"days\":{\"friday\":%d,\"monday\":%d,\"saturday\":%d,\"sunday\":%d,\"thursday\":%d,\"tuesday\":%d,\"wednesday\":%d}}", CheckDays.friday, CheckDays.monday, CheckDays.saturday, CheckDays.sunday, CheckDays.thursday, CheckDays.tuesday, CheckDays.wednesday);
                            }
                            strcat(TempBuffer, ScheduleParse.Days[index]);
                            // ESP_LOGE(TAG_SCHEDULE,"%s",ScheduleParse.Days[index]);
                            // ESP_LOGI(TAG_SCHEDULE,"[Append 464]: %d bytes AFTER | %s",strlen(TempBuffer),TempBuffer);
                        }
                        index++;
                    }
                    strcat(TempBuffer, "]}\n");
                    ESP_LOGI(TAG_SCHEDULE, "%s", TempBuffer);
                    // Check if destination file exists before renaming
                    ESP_LOGW(TAG_SCHEDULE, "Checking for File %s", ScheduleName);
                    struct stat st;
                    if (stat(ScheduleName, &st) == 0)
                    {
                        ESP_LOGI(TAG_SCHEDULE, "Creating Schedule file if not present ");
                        FILE *f = fopen(ScheduleName, "w");
                        if (f == NULL)
                        {
                            ESP_LOGE(TAG_SCHEDULE, "Failed to open file for writing");
                            return;
                        }
                        fprintf(f, TempBuffer);
                        fclose(f);
                    }
                    else
                    {
                        ESP_LOGI(TAG_SCHEDULE, "Creating Schedule file if not present ");
                        FILE *f = fopen(ScheduleName, "w");
                        if (f == NULL)
                        {
                            ESP_LOGE(TAG_SCHEDULE, "Failed to open file for writing");
                            return;
                        }
                        fprintf(f, TempBuffer);
                        fclose(f);
                    }
                    isCheckforSchedule = true;
                }
                else
                {
                    ESP_LOGE(TAG_SCHEDULE, "No Schedule to Store for %s", RelayIdentifier);
                }
            }
        }
    }
}

struct Schedule_h ParseTimeString(char *TimeString, struct tm info)
{
    struct Schedule_h time;
    char tempbuff[5];
    sprintf(tempbuff, "%c%c%c", TimeString[0], TimeString[1], TimeString[2]);
    strcpy(time.Day, tempbuff);
    // printf("%s\n",time.Day);
    memset(tempbuff, 0, sizeof(tempbuff));
    sprintf(tempbuff, "%c%c%c", TimeString[4], TimeString[5], TimeString[6]);
    time.Month = MonthToNumber(tempbuff);
    // printf("%d\n",time.Month);

    time.Date = info.tm_mday;
    // printf("%d\n",time.Date);

    time.Hour = info.tm_hour;
    // printf("%d\n",time.Hour);

    time.Minute = info.tm_min;
    // printf("%d\n",time.Minute);

    time.Year = 2000 + (info.tm_year - 100);
    // printf("%d\n",time.Year);

    return time;
}

int MonthToNumber(char *month)
{
    if (strcmp("Jan", month) == 0)
    {
        return 1;
    }
    else if (strcmp("Feb", month) == 0)
    {
        return 2;
    }
    else if (strcmp("Mar", month) == 0)
    {
        return 3;
    }
    else if (strcmp("Apr", month) == 0)
    {
        return 4;
    }
    else if (strcmp("May", month) == 0)
    {
        return 5;
    }
    else if (strcmp("Jun", month) == 0)
    {
        return 6;
    }
    else if (strcmp("Jul", month) == 0)
    {
        return 7;
    }
    else if (strcmp("Aug", month) == 0)
    {
        return 8;
    }
    else if (strcmp("Sep", month) == 0)
    {
        return 9;
    }
    else if (strcmp("Oct", month) == 0)
    {
        return 10;
    }
    else if (strcmp("Nov", month) == 0)
    {
        return 11;
    }
    else if (strcmp("Dec", month) == 0)
    {
        return 12;
    }
    return 0;
}

struct Schedule_h CheckDayinSchedule(char *Day)
{
    struct Schedule_h CheckDay;
    if (strcmp("Mon", Day) == 0)
    {
        strcpy(CheckDay.checkDay, "monday");
        return CheckDay;
    }
    else if (strcmp("Tue", Day) == 0)
    {
        strcpy(CheckDay.checkDay, "tuesday");
        return CheckDay;
    }
    else if (strcmp("Wed", Day) == 0)
    {
        strcpy(CheckDay.checkDay, "wednesday");
        return CheckDay;
    }
    else if (strcmp("Thu", Day) == 0)
    {
        strcpy(CheckDay.checkDay, "thursday");
        return CheckDay;
    }
    else if (strcmp("Fri", Day) == 0)
    {
        strcpy(CheckDay.checkDay, "friday");
        return CheckDay;
    }
    else if (strcmp("Sat", Day) == 0)
    {
        strcpy(CheckDay.checkDay, "saturday");
        return CheckDay;
    }
    else if (strcmp("Sun", Day) == 0)
    {
        strcpy(CheckDay.checkDay, "sunday");
        return CheckDay;
    }
    strcpy(CheckDay.checkDay, "invalid");
    return CheckDay;
}

struct Schedule_h GetCurrentTime()
{

    time_t now = 0;
    //timeinfo = {0};

    char strftime_buf[64] = {0};
    struct Schedule_h currentTime;
    time(&now);
    localtime_r(&now, &timeinfo);
    if (timeinfo.tm_year < (2016 - 1900)) {
        ESP_LOGI(TAG_SCHEDULE, "Time is not set yet. Connecting to WiFi and getting time over NTP.");
        obtain_time();
        // update 'now' variable with current time
        time(&now);
    }
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);

    ESP_LOGI(TAG_SCHEDULE, "The current date/time in Pakistan is: %s", strftime_buf);
    currentTime = ParseTimeString(strftime_buf, timeinfo);
    return currentTime;
}

int find_minimum(uint8_t a[], int n)
{
    int c, index = 0;

    for (c = 1; c < n; c++)
        if (a[c] < a[index])
            index = c;

    return index;
}

// Function to move all zeros present in the array to the end
void moveAllZeroesAtArrayEnd(uint8_t arr[], int n)
{
    // endOfNonZero stores index of next available position
    uint8_t endOfNonZero = 0;
    uint8_t i;
    for (i = 0; i < n; i++)
    {
        // if current element is non-zero, put the element at
        // next free position in the array
        if (arr[i] != 0)
        {
            arr[endOfNonZero++] = arr[i];
            // printf("Storing %d at index %d",arr[i],endOfNonZero-1);
        }
    }
    // move all 0's to the end of the array
    for (i = endOfNonZero; i < n; i++)
    {
        arr[i] = 0;
    }
}

int FindSimilarSchedulesSameHour(uint8_t a[], uint8_t b[], int n, int hour)
{
    int HoursCounter[15];
    int minARRAY[15];
    int index = 0;
    for (int i = 0; i < n; i++)
    {
        if (a[i] == hour)
        {
            HoursCounter[index] = i;
            index++;
        }
    }
    if (HoursCounter[0] == 0)
    {
        return FALSE;
    }

    index = 0;

    for (int c = 1; c < n; c++)
    {
        if (HoursCounter[c] != 0)
        {
            int copyLocation = HoursCounter[c];
            minARRAY[index] = b[copyLocation];
            index++;
        }
    }
    index = find_minimum(minARRAY, index - 1);
    return index;
}
