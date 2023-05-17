//------------includes ------------

#include "relay_modified.h"
#include "nvs.h"
#include "common.h"


#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_timer.h"

//------------------variables initailization--------------------
 
//to get current  state of relay 1 
bool relay1_open_state = false ; 

//to get current state of relay2 

bool relay2_open_state = false ; 


bool relay1_closed = true ; 

bool relay2_closed = true ; 

bool update1_if_pushed = false ; 

bool update2_if_pushed = false ; 
// device settings variables with default values in case nothing stored in flash

// relay1
bool ExtInput1 = true;

char *Name1 = "Front Gate";
char *Name2 = "Back Gate";


int tempAlert = 40;
bool alertOnClose = true;
bool alertOnOpen = true;
bool nightAlert = true;
char *region = "USA";


int Outtime1;
bool scheduled1 = false;
int autoClose1=0;
int reedswitch1;
// relay2
bool ExtInput2 = true;

int Outtime2;
bool scheduled2 = false;
int autoClose2=0;
int reedswitch2;

bool relay1_status = false;
bool relay2_status = false;

bool reed1_status_changed = false;
bool reed2_status_changed = false;
bool device_settings_updated = false;
int relay_1_io;
int relay_2_io;
bool now_read_flash = false;
int reed_switch_io_1;
int reed_switch_io_2;


bool enable_autoClose1 = false ; 
bool enable_autoClose2 = false ; 
//bool manual_actuator_relay_2_ON = false;

static void driver_init(int relay_0_gpio, int relay_1_gpio)
{
    /* Configure output */
    gpio_config_t io_conf = {
        .mode = GPIO_MODE_INPUT_OUTPUT,
        .pull_up_en = 0,
        .pull_down_en = 0,
    };
    io_conf.pin_bit_mask = ((uint64_t)1 << relay_0_gpio);
    io_conf.pin_bit_mask |= ((uint64_t)1 << relay_1_gpio);
    /* Configure the GPIO */
    gpio_config(&io_conf);
}

void update_device_settings()
{
    printf("\n\n\nREADING CONFIG SETTINGS FROM Flash\n\n\n");
    esp_err_t err;
    char *out_value = NULL;
    int temp;

    // common settings
    err = nvs_read("tempAlert", &out_value);
    if (err == ESP_OK)
    {
        sscanf(out_value, "%d", &tempAlert);
        printf("The Int for the num is %d\n", tempAlert);
    }

    err = nvs_read("alertOnClose", &out_value);
    if (err == ESP_OK)
    {
        sscanf(out_value, "%d", &temp);
        alertOnClose = (bool)temp;
        printf("The Int for the num is %d\n", alertOnClose);
    }

    err = nvs_read("alertOnOpen", &out_value);
    if (err == ESP_OK)
    {
        sscanf(out_value, "%d", &temp);
        alertOnOpen = (bool)temp;
        printf("The Int for the num is %d\n", alertOnOpen);
    }

    err = nvs_read("nightAlert", &out_value);
    if (err == ESP_OK)
    {
        sscanf(out_value, "%d", &temp);
        nightAlert = (bool)temp;
        printf("The Int for the num is %d\n", nightAlert);
    }

    err = nvs_read("region", &out_value);
    if (err == ESP_OK)
    {
        region = out_value;
    }

    // relay 1 settings
    err = nvs_read("ExtInput1", &out_value);
    if (err == ESP_OK)
    {
        sscanf(out_value, "%d", &temp);
        ExtInput1 = (bool)temp;
        printf("The Int for the num is %d\n", ExtInput1);
    }

    err = nvs_read("Outtime1", &out_value);
    if (err == ESP_OK)
    {
        sscanf(out_value, "%d", &Outtime1);
        printf("The Int for the num is %d\n", Outtime1);
    }

    err = nvs_read("schecduled1", &out_value);
    if (err == ESP_OK)
    {
        sscanf(out_value, "%d", &temp);
        scheduled1 = (bool)temp;
        printf("The Int for the num is %d\n", scheduled1);
    }

    err = nvs_read("autoClose1", &out_value);
    if (err == ESP_OK)
    {
        sscanf(out_value, "%d", &autoClose1);
        printf("The Int for the num is %d\n", autoClose1);
    }

    err = nvs_read("reedswitch1", &out_value);
    if (err == ESP_OK)
    {
        sscanf(out_value, "%d", &temp);
        reedswitch1 = (bool)temp;
        printf("The Int for the num is %d\n", reedswitch1);
    }

    err = nvs_read("Name1", &out_value);
    if (err == ESP_OK)
    {
        Name1 = out_value;
    }

    // relay 2 settings
    err = nvs_read("ExtInput2", &out_value);
    if (err == ESP_OK)
    {
        sscanf(out_value, "%d", &temp);
        ExtInput2 = (bool)temp;
        printf("The Int for the num is %d\n", ExtInput2);
    }

    err = nvs_read("Outtime2", &out_value);
    if (err == ESP_OK)
    {
        sscanf(out_value, "%d", &Outtime2);
        printf("The Int for the num is %d\n", Outtime2);
    }

    err = nvs_read("scheduled2", &out_value);
    if (err == ESP_OK)
    {
        sscanf(out_value, "%d", &temp);
        scheduled2 = (bool)temp;
        printf("The Int for the num is %d\n", scheduled2);
    }

    err = nvs_read("autoClose2", &out_value);
    if (err == ESP_OK)
    {
        sscanf(out_value, "%d", &autoClose2);
        printf("The Int for the num is %d\n", autoClose2);
    }

    err = nvs_read("reedswitch2", &out_value);
    if (err == ESP_OK)
    {
        sscanf(out_value, "%d", &temp);
        reedswitch2 = (bool)temp;
        printf("The Int for the num is %d\n", reedswitch2);
    }
    err = nvs_read("Name2", &out_value);
    if (err == ESP_OK)
    {
        Name2 = out_value;
    }
    printf("\n\n\nSUCCESSFULLY READ & UPDATED CONFIG SETTINGS FROM Flash\n\n\n");
}

void manual_relay_control(uint32_t relay_io_no, int relay_no)
{
    if (relay_no == 1 && ExtInput1 == true)
    {
        relay1_status = true;
    }
    else if (relay_no == 2 && ExtInput2 == true)
    {
        relay2_status = true;
    }
}

bool check_gate_status(int gate_no)
{
    bool status;
    if (gate_no == 1)
    {
        
        if (reedswitch1 == 1)
        {
            status = gpio_get_level(reed_switch_io_1);
        }
        else
        {
            
            //status = !(gpio_get_level(relay_1_io));
            status = relay1_open_state; 
        }
    }

    else
    {
        if (reedswitch2 == 1)
        {
            // gpio = reed_switch_io_2;
            status = gpio_get_level(reed_switch_io_2);
        }
        else
        {
            //status = !(gpio_get_level(relay_2_io));
            status = relay2_open_state;
        }
    }

    return status;
}

static void relay_task(void *arg)
{
    // updadte device settings variables from flash at start
    now_read_flash = false;
    update_device_settings();
    // default relay output values
    gpio_set_level(relay_1_io, 1); // High signal turns Relays OFF
    gpio_set_level(relay_2_io, 1);

    // bool relay_1_last_status = relay1_status;
    // bool relay_2_last_status = relay2_status;
    bool relay_1_timer = false;
    bool relay_2_timer = false;

    reedStatus1 = check_gate_status(1);
    bool reedLastStatus1 = reedStatus1;
    reedStatus2 = check_gate_status(2);
    bool reedLastStatus2 = reedStatus2;
    bool auto_close_cmd_R1 = false;
    bool auto_close_cmd_R2 = false;

    int64_t timeStart_relay_1 = 0;
    int64_t timeStart_relay_2 = 0;
    int64_t auto_close_time_R1 = 0;
    int64_t auto_close_time_R2 = 0;
    for (;;)
    {
        reedStatus1 = check_gate_status(1);                        // Check if gate opened/closed manually
        reedStatus2 = check_gate_status(2);                        // Check if gate opened/closed manually


        if (relay1_status == true )
        {
            
            if (relay1_open_state == false && relay1_closed == true ){
                printf("Changing relay1 state\n\n\n\n");
            gpio_set_level(relay_1_io, 0) ; // to trip relay 
            relay1_open_state = true ;
            //relay1_closed = false ;
            timeStart_relay_1 = esp_timer_get_time();
            update1_if_pushed = true ; 
            }
            else if (update1_if_pushed == true ){
                printf("\n\n \n Closing relay 1 \n\n\n\n");
                gpio_set_level(relay_1_io, 1) ; // to trip relay
                vTaskDelay(1000 / portTICK_PERIOD_MS);
                gpio_set_level(relay_1_io,0); 
                // int64_t dummy = timeStart_relay_1 ;
                // timeStart_relay_1 = esp_timer_get_time() - dummy ; 
                //relay1_closed = true ; 
                update1_if_pushed = false ; 
            }
            if (autoClose1 > 0){
                enable_autoClose1 = true ; 
            }
            else {
                enable_autoClose1 = false ; 
            }
            relay1_status = false;
            relay_1_timer = true;
            
        }
        if (relay2_status == true )
        {
            if (relay2_open_state == false && relay2_closed == true ){
                printf("Changing relay2 state\n\n\n\n");
            gpio_set_level(relay_2_io, 0) ; // to trip relay 
            relay2_open_state = true ;
            //relay1_closed = false ;
            timeStart_relay_2 = esp_timer_get_time();
            update2_if_pushed = true ; 
            }
            else if (update2_if_pushed == true ){
                printf("\n\n \n Closing relay 1 \n\n\n\n");
                gpio_set_level(relay_2_io, 1) ; // to trip relay 
                vTaskDelay(1000 / portTICK_PERIOD_MS);
                gpio_set_level(relay_2_io,0);
                // int64_t dummy = timeStart_relay_2 ;
                // timeStart_relay_2 = esp_timer_get_time() - dummy ; 
                //relay1_closed = true ; 
                update2_if_pushed = false ; 
            }

            if (autoClose2 > 0 ){
                enable_autoClose2 = true ; 
            }
            else {
                enable_autoClose2 = false ; 
            }
            relay2_status = false;
            relay_2_timer = true;
        }
        if (relay_1_timer == true)
        {
            if ((esp_timer_get_time() - timeStart_relay_1) >= (Outtime1 * 1000000))
            {
                gpio_set_level(relay_1_io, 1);
                relay1_open_state = false ;
                relay_1_timer = false;
                relay1_closed = true ; 
            }
        }
        if (relay_2_timer == true)
        {
            if ((esp_timer_get_time() - timeStart_relay_2) >= (Outtime2 * 1000000))
            {
                gpio_set_level(relay_2_io, 1);
                relay_2_timer = false;
                relay2_closed = true ;
                relay2_open_state = false ; 
            }
        }
        if (enable_autoClose1 == true && relay1_open_state == true )
        {
            if (auto_close_cmd_R1 == false) 
            {
                auto_close_cmd_R1 = true;
                auto_close_time_R1 = esp_timer_get_time();
            }
            if (auto_close_cmd_R1 == true)
            {
                if ((esp_timer_get_time() - auto_close_time_R1) >= (autoClose1 * 1000000))
                {
                
                    relay1_status = true;
                    relay1_open_state = false ; 
                    auto_close_cmd_R1 = false;
                }
            }
        }
        else
        {
            if (auto_close_cmd_R1 == true)
                auto_close_cmd_R1 = false;
        }
        if (enable_autoClose2 == true && relay2_open_state == true )
        {
            if (auto_close_cmd_R2 == false)
            {
                auto_close_cmd_R2 = true;
                auto_close_time_R2 = esp_timer_get_time();
            }
            if (auto_close_cmd_R2 == true)
            {
                if ((esp_timer_get_time() - auto_close_time_R2) >= (autoClose2 * 1000000))
                {
                    relay1_status = true ; 
                    relay1_open_state = false ; 
                    auto_close_cmd_R2 = false;
                }
            }
        }
        else
        {
            if (auto_close_cmd_R2 == true)
                auto_close_cmd_R2 = false;
        }

        // check device settings variables update here
        if (device_settings_updated == true)
        {
            update_device_settings();
            device_settings_updated = false;
        }

        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void relay_init(int relay_0_gpio, int relay_1_gpio, int reed_1_io, int reed_2_io)
{
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    printf("\n\n\nRelay Task Starting........................\n\n\n");
    relay_1_io = relay_0_gpio;
    relay_2_io = relay_1_gpio;
    reed_switch_io_1 = reed_1_io;
    reed_switch_io_2 = reed_2_io;
    driver_init(relay_1_io, relay_2_io);
    // create task
    xTaskCreate(relay_task, "relay_task", 2048, NULL, 1, NULL);
}