#ifndef _PAIRING_WIFI_H
#define _PAIRING_WIFI_H
#include <stdio.h>
#include <stdbool.h>
bool start_pairing_wifi();
void wifi_mode_sta_only();
void wifi_init_sta_ap(void); 
void get_mac_address();
char *wifi_list();
void stop_webserver(); 
bool check_wifi(); 

#endif