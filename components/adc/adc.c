#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "adc.h"

#define DEFAULT_VREF 1100 // Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES 64  // Multisampling
bool adc_gpio_ok = true;
int ADC_ENABLE ; 
enum ADC_GPIO_MAP
{
    // ADC1
    ADC_IO_36 = 0,
    ADC_IO_37 = 1,
    ADC_IO_38 = 2,
    ADC_IO_39 = 3,
    ADC_IO_32 = 4,
    ADC_IO_33 = 5,
    ADC_IO_34 = 6,
    ADC_IO_35 = 7,
    // ADC2
    ADC2_IO_4 = 0,
    ADC2_IO_0 = 1,
    ADC2_IO_2 = 2,
    ADC2_IO_15 = 3,
    ADC2_IO_13 = 4,
    ADC2_IO_12 = 5,
    ADC2_IO_14 = 6,
    ADC2_IO_27 = 7,
    ADC2_IO_25 = 8,
    ADC2_IO_26 = 9
};

enum ADC_GPIO_MAP ADC_GPIO;
static esp_adc_cal_characteristics_t *adc_chars;
static const adc_bits_width_t width = ADC_WIDTH_BIT_12;
// static const adc_atten_t atten = ADC_ATTEN_DB_0;
static const adc_atten_t atten = ADC_ATTEN_DB_11;
static const adc_unit_t unit = ADC_UNIT_1;
// adc_channel_t channel = ADC_CHANNEL_6; // GPIO34 if ADC1, GPIO14 if ADC2
adc_channel_t channel; // GPIO34 if ADC1, GPIO14 if ADC2

static void check_efuse(void)
{
    // Check if TP is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK)
    {
        printf("eFuse Two Point: Supported\n");
    }
    else
    {
        printf("eFuse Two Point: NOT supported\n");
    }
    // Check Vref is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) == ESP_OK)
    {
        printf("eFuse Vref: Supported\n");
    }
    else
    {
        printf("eFuse Vref: NOT supported\n");
    }
}

static void print_char_val_type(esp_adc_cal_value_t val_type)
{
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP)
    {
        printf("Characterized using Two Point Value\n");
    }
    else if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF)
    {
        printf("Characterized using eFuse Vref\n");
    }
    else
    {
        printf("Characterized using Default Vref\n");
    }
}

void adc_init(uint32_t gpio , uint32_t ADC_MEASUREMENT_EN, int adc_num )
{
    ADC_GPIO = 99;
    if (adc_num == 1)
    {
        if (gpio == 32)
        {
            ADC_GPIO = ADC_IO_32;
        }
        else if (gpio == 33)
        {
            ADC_GPIO = ADC_IO_33;
        }
        else if (gpio == 34)
        {
            ADC_GPIO = ADC_IO_34;
        }
        else if (gpio == 35)
        {
            ADC_GPIO = ADC_IO_35;
        }
        else if (gpio == 36)
        {
            ADC_GPIO = ADC_IO_36;
        }
        else if (gpio == 37)
        {
            ADC_GPIO = ADC_IO_37;
        }
        else if (gpio == 38)
        {
            ADC_GPIO = ADC_IO_38;
        }
        else if (gpio == 39)
        {
            ADC_GPIO = ADC_IO_39;
        }
    }
    else if (adc_num == 2)
    {
        if (gpio == 0)
        {
            ADC_GPIO = ADC2_IO_0;
        }
        else if (gpio == 2)
        {
            ADC_GPIO = ADC2_IO_2;
        }
        else if (gpio == 4)
        {
            ADC_GPIO = ADC2_IO_4;
        }
        else if (gpio == 12)
        {
            ADC_GPIO = ADC2_IO_12;
        }
        else if (gpio == 13)
        {
            ADC_GPIO = ADC2_IO_13;
        }
        else if (gpio == 14)
        {
            ADC_GPIO = ADC2_IO_14;
        }
        else if (gpio == 15)
        {
            ADC_GPIO = ADC2_IO_15;
        }
        else if (gpio == 25)
        {
            ADC_GPIO = ADC2_IO_25;
        }
        else if (gpio == 26)
        {
            ADC_GPIO = ADC2_IO_26;
        }
        else if (gpio == 27)
        {
            ADC_GPIO = ADC2_IO_27;
        }
    }

    if (ADC_GPIO == 99)
    {
        printf("Wrong ADC Pin selected. Cannot initialize ADC\n");
        adc_gpio_ok = false;
    }

    if (adc_gpio_ok == true)
    {
        printf("Channel selected is: %d\n", ADC_GPIO);
        channel = ADC_GPIO;

        // Check if Two Point or Vref are burned into eFuse
        check_efuse();

        // Configure ADC
        if (unit == ADC_UNIT_1)
        {
            adc1_config_width(width);
            adc1_config_channel_atten(channel, atten);
        }
        else
        {
            adc2_config_channel_atten((adc2_channel_t)channel, atten);
        }

        // Characterize ADC
        adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
        esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit, atten, width, DEFAULT_VREF, adc_chars);
        print_char_val_type(val_type);
        /* Configure output */
        gpio_config_t io_conf = {
            .mode = GPIO_MODE_INPUT_OUTPUT,
            .pull_up_en = 0,
            .pull_down_en = 0,
        };
        io_conf.pin_bit_mask = ((uint64_t)1 << ADC_MEASUREMENT_EN);
        /* Configure the GPIO */
        gpio_config(&io_conf);
        ADC_ENABLE = ADC_MEASUREMENT_EN ; 
    }
}

uint32_t get_adc_reading()
{
    if (adc_gpio_ok == true)
    {
        gpio_set_level(ADC_ENABLE, 1); // High signal enables battery measurement path through MOSFET. 
        uint32_t adc_reading = 0;
        // Multisampling
        for (int i = 0; i < NO_OF_SAMPLES; i++)
        {
            if (unit == ADC_UNIT_1)
            {
                adc_reading += adc1_get_raw((adc1_channel_t)channel);
            }
            else
            {
                int raw;
                adc2_get_raw((adc2_channel_t)channel, width, &raw);
                adc_reading += raw;
            }
        }
        adc_reading /= NO_OF_SAMPLES;
        // Convert adc_reading to voltage in mV
        uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
        // float voltage = (adc_reading / 4095.0) * 2 * 1.1 * 3.3;
        printf("Raw: %d\t Voltage %d\n", adc_reading, voltage);

        gpio_set_level(ADC_ENABLE, 0); // LOW signal DISABLES battery measurement path through MOSFET.
        return voltage;
    }
    else
    {
        return 0;
    }
}
