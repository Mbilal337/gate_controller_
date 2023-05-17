#ifndef _NVS_FLASH_H
#define _NVS_FLASH_H

#include <stdio.h>
#include "nvs_flash.h"

nvs_handle_t my_handle;

void nvs_begin();
esp_err_t nvs_read(char * key, char ** out_value);
esp_err_t nvs_write(char * key, char ** value);
#endif