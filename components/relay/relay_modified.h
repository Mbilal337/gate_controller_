#ifndef _RELAY_MODIFIED_H
#define _RELAY_MODIFIED_H

#include <stdio.h>
#include <stdbool.h>

void relay_init(int, int, int, int);
void manual_relay_control(uint32_t relay_io_no, int relay_no);
bool check_gate_status(int reed_switch);
#endif