#ifndef _PAIRING_BLE_H
#define _PAIRING_BLE_H
#include <stdio.h>
#include <stdbool.h>
bool start_pairing_ble(void);
bool stop_pairing_ble(void);
void ble_app_advertise(void);
int stop_advertise(void);
#endif