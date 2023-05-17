#include "pairing.h"
#include "common.h"

// BLE OR WIFI pairing mode
#ifdef wifi_ble_both
#include "pairing_ble.h"
#include "pairing_wifi.h"
#endif

#ifdef only_ble
#include "pairing_ble.h"
#endif

#ifdef only_wifi
#include "pairing_wifi.h"
#endif
//...........
#include "nvs_flash.h"
#include "esp_system.h"
int status_code = 404;
bool credential_recieved = false;
bool wifi_connected = false;
bool device_registered = false;
bool device_id_acquired = false;
bool pairing_started = false ; 
bool device_connected = false ; 
/// @brief
//          Function to start the pairing mode
/// @param
//      None
/// @return
//      True if pairing started else false
bool start_pairing(void)
{
    wifi_connected = false;
    credential_recieved = false;
    device_id_acquired = false;
    device_registered = false;
#ifdef only_ble
    return start_pairing_ble();
#endif
#ifdef only_wifi
    return start_pairing_wifi();
#endif
#ifdef wifi_ble_both
    start_pairing_wifi();
    return start_pairing_ble();
#endif
}

bool stop_pairing(void)
{
#ifdef only_ble
    // stop BLE and clear memory or end tasks if any and start STA
    bool state = stop_pairing_ble();
    if (state)
    {
        wifi_mode_sta_only();
    }
    return state;

#endif
#ifdef only_wifi
    // Stop WiFi ap mode and start only STA
    wifi_mode_sta_only();
    return true;

#endif
#ifdef wifi_ble_both
    bool state = stop_pairing_ble();

    if (state)
    {
        printf("ble stopped \n\n"); 
        // wifi_mode_sta_only();
    }
    return state;

// stop both ble and wifi and start STA
#endif
}