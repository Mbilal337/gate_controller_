#ifndef _ADC_H
#define _ADC_H

#include <stdio.h>

void adc_init(uint32_t gpio, uint32_t ADC_MEASUREMENT_EN , int adc_num);
uint32_t get_adc_reading();

#endif