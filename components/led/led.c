
#include <stdio.h>
#include <stdbool.h>
#include "driver/ledc.h"
#include "esp_err.h"
#include "math.h"
#include "led.h"
#include "esp_log.h"
// SemaphoreHandle_t xSemaphore_LED_BLINK;

#define TAG "LED_TASK"

#define LEDC_LS_TIMER LEDC_TIMER_0
#define LEDC_LS_MODE LEDC_LOW_SPEED_MODE

// Status Leds
uint32_t duty_bits = 13;
uint32_t freq = 3;
uint32_t num_leds = 3;
// #if HARDWARE_VERSION == HARD_V_1
// int leds[] = {21, 22, 23};
// #elif HARDWARE_VERSION == HARD_V_2
// int leds[] = {5, 18, 19};
// #endif

// int leds[] = {32, 34, 33,18,19,13}; //LED_1_RED , LED_1_GREEN , LED_1_BLUE,LED_2_RED , LED_2_GREEN , LED_2_BLUE
int leds[] = {18, 19, 13}; // LED_1_RED , LED_1_GREEN , LED_1_BLUE,LED_2_RED , LED_2_GREEN , LED_2_BLUE
// GPIO34 problem ------------
// int leds[] = {15,2, 4,18,19,13} ;
bool out_of_range = false;
ledc_timer_config_t my_ledc_timer;
ledc_channel_config_t *ledc_channel_array = NULL; // malloc assigned, when to free?
// SemaphoreHandle_t xSemaphore_LED_BLINK = NULL;
uint32_t num_of_leds_global = 0;

void led_init(uint32_t duty_resolution_bits, uint32_t freq_hz, uint32_t num_of_leds, int *leds)
{

    if (num_of_leds < 8)
    {
        if (out_of_range == true)
        {
            out_of_range = false;
        }

        ledc_timer_config_t ledc_timer = {
            .duty_resolution = duty_resolution_bits, // resolution of PWM duty
            .freq_hz = freq_hz,                      // frequency of PWM signal
            .speed_mode = LEDC_LS_MODE,              // timer mode
            .timer_num = LEDC_LS_TIMER,              // timer index
            .clk_cfg = LEDC_AUTO_CLK,                // Auto select the source clock
        };

        my_ledc_timer = ledc_timer;
        // Set configuration of timer0 for high speed channels
        ledc_timer_config(&my_ledc_timer);

        size_t led_array_length = sizeof(ledc_channel_config_t) * num_of_leds;
        ledc_channel_config_t *ledc_channel = malloc(led_array_length); // malloc assigned, when to free?

        for (int i = 0; i < num_of_leds; i++)
        {
            ledc_channel[i].channel = (ledc_channel_t)i;
            ledc_channel[i].duty = 0;
            ledc_channel[i].gpio_num = leds[i];
            ledc_channel[i].speed_mode = LEDC_LS_MODE;
            ledc_channel[i].hpoint = 0;
            ledc_channel[i].timer_sel = LEDC_LS_TIMER;
            ledc_channel[i].intr_type = LEDC_INTR_DISABLE;
        }
        ledc_channel_array = ledc_channel;
        printf("Size: %d\n", led_array_length);
        printf("Timer = %d\n", (uint32_t)ledc_channel_array[0].speed_mode);
        printf("Timer1 = %d\n", (uint32_t)ledc_channel_array[2].speed_mode);
        // Set LED Controller with previously prepared configuration
        for (int ch = 0; ch < num_of_leds; ch++)
        {
            ledc_channel_config(&ledc_channel_array[ch]); // channel 0: RED,1:GREEN , 2: BLUE
        }
    }
    else
    {
        printf("Total Number of Leds Exceeded Limit of 7. You wont be able to use LED functions");
        out_of_range = true;
    }
    xSemaphore_LED_BLINK1 = xSemaphoreCreateMutex();
    xSemaphore_LED_BLINK2 = xSemaphoreCreateMutex();
    num_of_leds_global = num_of_leds;
}

int led_change_duty(uint32_t channel, uint32_t duty)
{
    if (channel < 3)
    {
        if (xSemaphore_LED_BLINK1 != NULL)
        {
            if (xSemaphoreTake(xSemaphore_LED_BLINK1, (TickType_t)0) == pdTRUE)
            {
                printf("LED Semaphore Taken\n");
                if (out_of_range == false)
                {
                    uint32_t duty_converted = pow(2, (int)my_ledc_timer.duty_resolution);
                    float duty_final = duty_converted * duty * 0.01;
                    ledc_set_duty(ledc_channel_array[channel].speed_mode, ledc_channel_array[channel].channel, (uint32_t)duty_final);
                    ledc_update_duty(ledc_channel_array[channel].speed_mode, ledc_channel_array[channel].channel);
                    printf("duty value: %d\n", (uint32_t)duty_final);
                }
                return 1;
            }
            else
            {
                printf("LED Semaphore already occupied\n");
            }
        }
        return 0;
    }
    else
    {
        if (xSemaphore_LED_BLINK2 != NULL)
        {
            if (xSemaphoreTake(xSemaphore_LED_BLINK2, (TickType_t)0) == pdTRUE)
            {
                printf("LED Semaphore Taken\n");
                if (out_of_range == false)
                {
                    uint32_t duty_converted = pow(2, (int)my_ledc_timer.duty_resolution);
                    float duty_final = duty_converted * duty * 0.01;
                    ledc_set_duty(ledc_channel_array[channel].speed_mode, ledc_channel_array[channel].channel, (uint32_t)duty_final);
                    ledc_update_duty(ledc_channel_array[channel].speed_mode, ledc_channel_array[channel].channel);
                    printf("duty value: %d\n", (uint32_t)duty_final);
                }
                return 1;
            }
            else
            {
                printf("LED Semaphore already occupied\n");
            }
        }
        return 0;
    }
}

void led_change_freq_hz(uint32_t channel, uint32_t freq_hz)
{
    if (out_of_range == false)
    {
        printf("Here ............\n");
        ledc_set_freq(ledc_channel_array[channel].speed_mode, ledc_channel_array[channel].timer_sel, freq_hz);
        printf("Timer = %d\n", (uint32_t)ledc_channel_array[channel].timer_sel);
    }
}

void led_stop_blinking(uint32_t channel, uint32_t final_value)
{
    if (out_of_range == false)
    {
        ledc_stop(ledc_channel_array[channel].speed_mode, ledc_channel_array[channel].channel, final_value);
    }
}

void led_stop_blinking_all(uint32_t final_value)
{
    if (out_of_range == false)
    {
        for (int i = 0; i < num_of_leds_global; i++)
        {
            ledc_stop(ledc_channel_array[i].speed_mode, ledc_channel_array[i].channel, final_value);
            // ledcWrite(ledc_channel_array[i].channel,final_value);
        }
    }
}

// void app_main(void)
// {
//     uint32_t duty_bits = 13;
//     uint32_t freq = 2;
//     uint32_t num_leds = 3;
//     int leds[] = {21, 22, 23};
//     led_init(duty_bits, freq, num_leds, leds);

//     uint32_t ch = 0;
//     uint32_t duty = 50;
//     led_change_duty(ch, duty);
//     vTaskDelay(3000 / portTICK_PERIOD_MS);
//     freq = 1;
//     led_change_freq_hz(ch, freq);
//     vTaskDelay(3000 / portTICK_PERIOD_MS);
//     led_stop_blinking(ch, 0);
//     vTaskDelay(3000 / portTICK_PERIOD_MS);
//     led_change_duty(ch, duty);

// }
static void led_task(void *arg)
{
    led_init(duty_bits, freq, num_leds, leds);

    for (;;)
    {
    }
}

void led_task_begin()
{
    ESP_LOGI(TAG, "\n\n ..........LED TASK STARTING ........ \n\n");
    // xTaskCreate(led_task, "led_task", 2048, NULL, 10, NULL);
    // num_leds = sizeof(leds);
    ESP_LOGI(TAG, "Number of leds : %d \n ", num_leds);
    led_init(duty_bits, freq, num_leds, leds);
}