#ifndef _COMMON_H
#define _COMMON_H
#include "esp_wifi.h"
#define wifi_ble_both 1 // only_wifi ,only_ble, wifi_ble_both

extern int status_code;
extern bool credential_recieved;
extern bool wifi_connected;
extern bool device_registered;
extern bool device_id_acquired;
extern bool Operation_Mode; // true = Pairing_mode , false = NOrmal_mode
extern bool pairing_started;
extern bool device_connected;
extern int retry;
extern char device_uuid[30]; // MAC ADDRESS without colons
extern wifi_config_t wifi_config_st;
extern wifi_config_t wifi_config_ap;
extern bool pairing_completed;
struct Credentials
{
    char param_SSID[32];
    char param_Password[32];
    char offset[32];
    char region[32];
} wifi_credentials;
#endif