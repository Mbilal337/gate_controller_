#include "wifi.h"
#include "pairing.h"
#include "common.h"
#include "led.h"

#include <stdlib.h>
#include <stdio.h>
#include "esp_system.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "nvs.h"

bool Operation_Mode = true; // true = Pairing_mode , false = NOrmal_mode
bool button_task_ap_mode = false;
static const char *TAG = "_WIFI_TASK_";
int64_t timeStart_pairing = 0;
static void wifi_task_example(void *arg)
{
    int ret = 0;
    int n = 0;
    if (Operation_Mode == true)
    {

        nvs_begin();
        timeStart_pairing = esp_timer_get_time();
        pairing_started = true;
        start_pairing();
        // led_stop_blinking_all(1);
        // ret = led_change_duty(1, 50); // Blinking LED_1_RED /.5 sec
    }

    while (1)
    {
        if (((esp_timer_get_time() - timeStart_pairing) >= 30000000) && pairing_started == true && device_connected != true)
        {
            // Operation_Mode = false;
            pairing_started = false;
            // led_stop_blinking(1, 0);
            // ret = led_change_duty(2, 50); // Blinking LED_1_GREEN /.5 sec
            bool stop = stop_pairing();
            if (stop)
            {
                ESP_LOGI(TAG, "Stoping pairing mode as no device connected in 5 sec \n\n");
            }
            else
            {
                ESP_LOGI(TAG, "UNABLE TO STOP PAIRING METHODS even if no device was connected in 5 sec \n\n");
            }
        }
        else if (device_connected == true && n == 0)
        {
            n++;
            // xSemaphoreGive(xSemaphore_LED_BLINK);
            // led_stop_blinking(1, 0);
            // ret = led_change_duty(1, 20); // Blinking LED_1_GREEN /.5 sec
        }
        if (pairing_completed == true && Operation_Mode == true)
        {
            Operation_Mode = false;
            // xSemaphoreGive(xSemaphore_LED_BLINK);
            // led_stop_blinking_all(0);
            // led_stop_blinking(1, 0);
            // ret = led_change_duty(2, 50); // Blinking LED_1_GREEN /.5 sec
            ESP_LOGI(TAG,"Pairing has completed \n"); 
            bool stop = stop_pairing();
            if (stop)
            {

                ESP_LOGI(TAG, "Starting Normal Mode as Pairing has completed %s\n\n", wifi_credentials.param_SSID);
            }
            else
            {

                ESP_LOGI(TAG, "UNABLE TO STOP PAIRING METHODS \n\n");
            }
        }
        if (button_task_ap_mode == true)
        {
            n = 0;
            // xSemaphoreGive(xSemaphore_LED_BLINK);
            button_task_ap_mode = false;
            pairing_completed = false;
            start_pairing();
            // led_stop_blinking_all(0);     // no blinking if disconnect in ap mode
            // ret = led_change_duty(1, 50); // Blinking LED_1_RED /.5 sec
        }
        if (wifi_connected == false && credential_recieved == true ){
            ESP_LOGI(TAG,"rety to connect wifi to main wifi\n\n"); 
            

        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void init_wifi_task()
{
    printf("\n\n\nWifi Task Starting........................\n\n\n");
    ESP_LOGI(TAG, "accessing wifi ssid inside wifi task : %s \n", (char *)wifi_config_st.sta.ssid);

    xTaskCreate(wifi_task_example, "wifi_task_example", 4096, NULL, 10, NULL);
}