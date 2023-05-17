#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "wifi.h"
#include "led.h"
#include "sht40.h"
#include "adc.h"
#include "button.h"
#include "aws.h"
#include "nvs.h"
#include "relay_modified.h"
static const char *TAG_MAIN = "MAIN";
// TOTAL GPIOS USED = 15 

/*INPUT - 6 GPIOS
    -Relay1_button-----------------GPIO27
    -Relay2_button ----------------GPIO14
    -GP_button --------------------GPIO23
    -BATT_MEASURE - ADC  ----------GPIO35
    -Reed Switch 1  ---------------GPIO36
    -Reed Switch 2 ----------------GPIO39
*/
#define GPIO_INPUT_IO_0 23
#define RELAY_INPUT_IO_0 27
#define RELAY_INPUT_IO_1 14
#define REED_INPUT_IO_0 36
#define REED_INPUT_IO_1 39

/*
  OUTPUT - 9 GPIOS
    -LED1 -RGB 3 GPIOs
    -LED2 -RGB 3 GPIOs
    -BATT_MEASURE_ENABLE
    -RELAY1
    -RELAY2

*/
#define GPIO_RELAY_OUT_0 27
#define GPIO_RELAY_OUT_1 14

uint32_t buttons[] = {GPIO_INPUT_IO_0, RELAY_INPUT_IO_0, RELAY_INPUT_IO_1, REED_INPUT_IO_0, REED_INPUT_IO_1};

// ADC
#define ADC_IO 35
#define ADC_EN_IO 4
#define ADC_NUMBER 1
int64_t timeStart_adc = 0;
int64_t duration_adc;

void button_short_pressed_cb(uint16_t duration, uint32_t button_no)
{
    printf("Button %d Pressed, duration : %d\n\n\n", button_no, duration);
    if (button_no == GPIO_INPUT_IO_0)
    {
        if (duration >= 1 && duration <= 3)
        {
            // add mutex, take here and only give after switching to station mode once credentials written to flash
            // or use mutex at flash only
            printf("Button %d Pressed, duration : %d\n\n\n", button_no, duration);
            // switch_wifi_mode_to_ap();
        }

#ifdef DEBUG_ON
        else if (duration > 3 && duration <= 6)
        {
            printf("Button %d Pressed, duration : %d\n\n\n", button_no, duration);
            // switch_wifi_mode_to_station();
        }
        else if (duration > 6)
        {
            // wifi_ping_failed(); // Just demo, not implemented
        }
#endif
    }

    else if (button_no == RELAY_INPUT_IO_0)
    {
        manual_relay_control(RELAY_INPUT_IO_0, 1);
    }

    else if (button_no == RELAY_INPUT_IO_1)
    {
        manual_relay_control(RELAY_INPUT_IO_1, 2);
    }
}

void app_main(void)
{

    int ret = 0;
    button_init(buttons, sizeof(buttons) / sizeof(buttons[0]), button_short_pressed_cb);
    led_task_begin();
    led_stop_blinking_all(1);
    init_wifi_task();
    float temperature = 0;
    float Humidity = 0;
    SHT40_Init();
    adc_init(ADC_IO, ADC_EN_IO, ADC_NUMBER);
    relay_init(GPIO_RELAY_OUT_0, GPIO_RELAY_OUT_1, REED_INPUT_IO_0, REED_INPUT_IO_1);
    ESP_LOGI(TAG_MAIN, "I2C initialized successfully\n");
    timeStart_adc = esp_timer_get_time();

    uint32_t voltage; // for battery

        // while (true)
    // {

    //     duration_adc = esp_timer_get_time() - timeStart_adc;
    //     vTaskDelay(100 / portTICK_PERIOD_MS);
    //     if (duration_adc >= 30000000) // checking adc every 30 sec
    //     {

    //         printf("ADC ----------------\n");
    //         voltage = (get_adc_reading() * 2) / 1; // voltage divider - R1 = 100K , R2 = 100K
    //         printf("Voltage: %dmV\n", voltage);
    //         if (voltage > 3600)
    //         { // voltage above 3.6 -- blinking green
    //             ESP_LOGI(TAG_MAIN, "INSIDE > 3600mV \n");
    //             led_stop_blinking(0, 1);
    //             led_stop_blinking(1, 1);
    //             led_stop_blinking(2, 1);
    //             vTaskDelay(100 / portTICK_PERIOD_MS);
    //             ret = led_change_duty(1, 50);
    //             if (ret == 1)
    //             {
    //                 xSemaphoreGive(xSemaphore_LED_BLINK);
    //                 printf("Led Semaphore Given by Main Task\n");
    //                 // duty_change_last_call = LED_DUTY_GREEN;
    //             }
    //         }
    //         else if (voltage >= 3200)
    //         { // voltage 3.2 - 3.6 -- blinking blue
    //             ESP_LOGI(TAG_MAIN, "INSIDE > 3200mV \n");
    //             led_stop_blinking(0, 1);
    //             led_stop_blinking(1, 1);
    //             led_stop_blinking(2, 1);
    //             vTaskDelay(100 / portTICK_PERIOD_MS);
    //             ret = led_change_duty(2, 50);
    //             if (ret == 1)
    //             {
    //                 xSemaphoreGive(xSemaphore_LED_BLINK);
    //                 printf("Led Semaphore Given by Main Task\n");
    //                 // duty_change_last_call = LED_DUTY_GREEN;
    //             }
    //         }
    //         else
    //         { // voltage less than 3.2 -- blinking red
    //             ESP_LOGI(TAG_MAIN, "INSIDE < 3200mV \n");
    //             led_stop_blinking(0, 1);
    //             led_stop_blinking(1, 1);
    //             led_stop_blinking(2, 1);
    //             vTaskDelay(100 / portTICK_PERIOD_MS);
    //             ret = led_change_duty(0, 50);
    //             if (ret == 1)
    //             {
    //                 xSemaphoreGive(xSemaphore_LED_BLINK);
    //                 printf("Led Semaphore Given by Main Task\n");
    //                 // duty_change_last_call = LED_DUTY_GREEN;
    //             }
    //         }
    //         timeStart_adc = esp_timer_get_time();
    //     }
    // }
}
