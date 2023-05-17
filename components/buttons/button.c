#include "common.h"
#include "led.h"
#include "button.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_timer.h"

typedef bool FLAG;
FLAG startTimer = false;
int64_t timeStart;
int64_t duration;

static void (*button_pressed_cb)(uint16_t, uint32_t) = NULL;

#define ESP_INTR_FLAG_DEFAULT 0

static xQueueHandle gpio_evt_queue = NULL;

static void IRAM_ATTR gpio_isr_handler(void *arg)
{
    uint32_t gpio_num = (uint32_t)arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

static void gpio_task_example(void *arg)
{
    uint32_t io_num;
    for (;;)
    {
        if (xQueueReceive(gpio_evt_queue, &io_num, 100)) // portMAX_DELAY
        {

            if ((gpio_get_level(io_num) == 0) && (startTimer == false))
            {
                startTimer = true;
                timeStart = esp_timer_get_time();
            }
            else if (gpio_get_level(io_num) == 1)
            {
                if (startTimer == true)
                {
                    duration = esp_timer_get_time() - timeStart;

                    if (duration >= 30000)
                    {
                        button_pressed_cb((duration / 1000000), io_num);
                    }
                    startTimer = false;
                }
            }
        }
        vTaskDelay(300 / portTICK_PERIOD_MS);
    }
}

void button_init(uint32_t *buttons, uint32_t noofbuttons, void (*button_cb)(uint16_t, uint32_t))
{
    printf("\n\n\nButton Task Starting........................\n\n\n");
    gpio_config_t io_conf;
    if (buttons != NULL)
    {
        if (button_cb != NULL)
        {
            button_pressed_cb = button_cb;
        }

        // install gpio isr service
        gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);

        for (uint8_t i = 0; i < noofbuttons; i++)
        {
            // interrupt of rising edge
            io_conf.intr_type = GPIO_INTR_ANYEDGE;
            // bit mask of the pins, use GPIO4/5 here
            io_conf.pin_bit_mask = (1ULL << buttons[i]);
            // set as input mode
            io_conf.mode = GPIO_MODE_INPUT;
            // enable pull-up mode
            io_conf.pull_up_en = 1;
            io_conf.pull_down_en = 0;
            // hook isr handler for specific gpio pin
            if (buttons[i] != 36 && buttons[i] != 39)
            {
                gpio_config(&io_conf);
                gpio_isr_handler_add(buttons[i], gpio_isr_handler, (void *)buttons[i]);
            }
            else
            {
                io_conf.pull_up_en = 0;
                gpio_config(&io_conf);
                // gpio_isr_handler_add(buttons[i], gpio_isr_handler, (void *)buttons[i]);
            }
        }
        // create a queue to handle gpio event from isr
        gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
        // start gpio task
        xTaskCreate(gpio_task_example, "gpio_task_example", 2048, NULL, 1, NULL);
    }
}