#ifndef _BUTTON_H
#define _BUTTON_H

#include <stdio.h>

void button_init(uint32_t *buttons, uint32_t noofbuttons, void (*button_cb)(uint16_t, uint32_t));

#endif