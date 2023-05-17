#ifndef _LED_H
#define _LED_H

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
// as there are 2 RGB LEDs we would need 2 Semaphores as their flow is independent of each other 
SemaphoreHandle_t xSemaphore_LED_BLINK1;  // RGB1 - to show battery status 
SemaphoreHandle_t xSemaphore_LED_BLINK2 ; // RGB2 - to show Network Status 

void led_init(uint32_t duty_resolution_bits, uint32_t freq_hz, uint32_t num_of_leds, int *leds);
int led_change_duty(uint32_t channel, uint32_t duty);
void led_change_freq_hz(uint32_t channel, uint32_t freq_hz);
void led_stop_blinking(uint32_t channel, uint32_t final_value);
void led_stop_blinking_all(uint32_t final_value);
void led_task_begin() ; 
#endif

// duty_resolution_bits -   From 1 to 20
// freq_hz              -   In Hertz
// num_of_leds          -   Max 7
// leds                 -   Array of Integers, Total elements equal to num_of_leds
// channel              -   Max 7, equal to num_of_leds, From 0 to num_of_leds -1
// duty                 -   In percentage, from 0 to 100, for full time on use 100
// final_value          -   0 or 1
// led_change_duty()    -   Used to Start/Initialize or Resume led after led_stop_blinking() function called. 
                            // Returns 1 if Semaphore taken and 0 if not.