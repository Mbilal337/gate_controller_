#include "nvs.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "nvs_flash.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

SemaphoreHandle_t xSemaphore_NVS_WRITE_READ = NULL;
void nvs_begin();

void nvs_begin()
{
    // Initialize NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    // Open
    printf("FLASH INITIALIZED \n");
    // printf("Opening Non-Volatile Storage (NVS) handle... ");
    // err = nvs_open("storage", NVS_READWRITE, &my_handle);
    // if (err != ESP_OK)
    // {
    //     printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    // }
    // else
    // {
    //     printf("Done\n");
    // }
    xSemaphore_NVS_WRITE_READ = xSemaphoreCreateMutex();
}

esp_err_t nvs_read(char *key, char **out_value)
{
    // Read
    if (xSemaphore_NVS_WRITE_READ != NULL)
    {
        if (xSemaphoreTake(xSemaphore_NVS_WRITE_READ, (TickType_t)0) == pdTRUE) // should i wait indefinitely since we have to write eventually?
        {
            printf("NVS Read Semaphore Taken\n");
            esp_err_t err;
            size_t required_size;
            err = nvs_get_str(my_handle, key, NULL, &required_size);
            *out_value = (char *)malloc(required_size);
            err = nvs_get_str(my_handle, key, *out_value, &required_size);
            switch (err)
            {
            case ESP_OK:
                printf("Done\n");
                printf("%s = %s\n", key, *out_value);
                break;
            case ESP_ERR_NVS_NOT_FOUND:
                printf("The value is not initialized yet!\n");
                break;
            default:
                printf("Error (%s) reading!\n", esp_err_to_name(err));
            }
            xSemaphoreGive(xSemaphore_NVS_WRITE_READ);
            printf("NVS Read Semaphore Released\n");
            return err;
        }
    }
    return ESP_ERR_INVALID_ARG; // Also what to return, expected esp_err_t type return?
}

esp_err_t nvs_write(char *key, char **value)
{
    // Write
    if (xSemaphore_NVS_WRITE_READ != NULL)
    {
        if (xSemaphoreTake(xSemaphore_NVS_WRITE_READ, (TickType_t)0) == pdTRUE) // should i wait indefinitely since we have to write eventually?
        {
            printf("NVS Write Semaphore Taken\n");
            esp_err_t err;
            printf("%s is: %s \n", key, *value);
            if ((*value != NULL) && (*value[0] != '\0'))
            {
                printf("Updating %s in NVS ... ", key);
                err = nvs_set_str(my_handle, key, *value);
                printf((err != ESP_OK) ? "Failed!\n" : "Done\n");

                // Commit written value.
                // After setting any values, nvs_commit() must be called to ensure changes are written
                // to flash storage. Implementations may write to storage at other times,
                // but this is not guaranteed.
                printf("Committing updates in NVS ... ");
                err = nvs_commit(my_handle);
                printf((err != ESP_OK) ? "Failed!\n" : "Done\n");
                xSemaphoreGive(xSemaphore_NVS_WRITE_READ);
                printf("NVS Write Semaphore Released\n");
                return err;
                // nvs_close(my_handle);
            }
            return ESP_ERR_INVALID_ARG;
        }
        else
        {
            // What to do?
        }
    }
    return ESP_ERR_INVALID_ARG; // Also what to return, expected esp_err_t type return?
}
