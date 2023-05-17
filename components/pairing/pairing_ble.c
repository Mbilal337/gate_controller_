#include "pairing_ble.h"
#include "common.h"
#include "pairing_wifi.h"

// BLE
#include "esp_nimble_hci.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "sdkconfig.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "freertos/queue.h"
#include "cJSON.h"
QueueHandle_t queue;
static const char *TAG_BLE = "Pairing_Mode_BLE";
bool pairing_completed = false;
#define DEVICE_ADVERTISE_NAME "Gate_Controller_a1112"

uint8_t ble_addr_type;
void ble_app_advertise(void);
char *resp;

// Write data to ESP32 defined as BLE server Handler
static int device_write(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    // printf("Data from the client: %.*s\n", ctxt->om->om_len, ctxt->om->om_data);
    char Buffer[70];
    if (strncmp((char *)ctxt->om->om_data, "{", strlen("{")) == 0)
    {
        ESP_LOGI(TAG_BLE, "recived the ssid and pas \n %s\n", (char *)ctxt->om->om_data);
        cJSON *root = cJSON_Parse((char *)ctxt->om->om_data);
        int root_array_size = cJSON_GetArraySize(root);
        ESP_LOGI(TAG_BLE, "items in cJSON are %d ", root_array_size);
        if (root_array_size > 0)
        {
            cJSON *element = cJSON_GetArrayItem(root, 0);
            if (element->type == cJSON_String)
            {
                ESP_LOGI(TAG_BLE, "ssid= %s", element->valuestring);
                strcpy((char *)wifi_config_st.sta.ssid, element->valuestring);
                printf("ssid is: %s", (char *)wifi_config_st.sta.ssid);
            }

            element = cJSON_GetArrayItem(root, 1);
            if (element->type == cJSON_String)
            {
                ESP_LOGI(TAG_BLE, "pass= %s", element->valuestring);
                strcpy((char *)wifi_config_st.sta.password, element->valuestring);
                printf("password is: %s", (char *)wifi_config_st.sta.password);
                credential_recieved = true;
            }
            element = cJSON_GetArrayItem(root, 2);
            if (element->type == cJSON_String)
            {
                ESP_LOGI(TAG_BLE, "offset= %s", element->valuestring);
            }

            wifi_mode_sta_only();
            sprintf(Buffer, "credentials");
            ESP_LOGI(TAG_BLE, "BUFFER IS WRITTEN WITH : %s \n ", Buffer);
            xQueueOverwrite(queue, (void *)Buffer);
            // if (device_id_acquired == false)
            // {
            //     get_mac_address(); // acquire device id
            //     device_id_acquired = true;
            // }
        }
        cJSON_Delete(root);
    }
    else
    {
        snprintf(Buffer, ctxt->om->om_len + 1, "%s", (char *)ctxt->om->om_data);
        ESP_LOGI(TAG_BLE, "BUFFER IS WRITTEN WITH : %s \n ", Buffer);
        xQueueOverwrite(queue, (void *)Buffer);
    }

    return 0;
}

// Read data from ESP32 defined as BLE server Handler
static int device_read(uint16_t con_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)

{
    char tx[70];
    if (xQueueReceive(queue, &tx, 100))
    { // portMAX_DELAY
        // ESP_LOGI(TAG_BLE, "DATA FROM QUEUE %s \n", tx);
        if(strcmp(tx,"stop_ble") == 0){
            ESP_LOGI(TAG_BLE,"STOPPING BLE CONNECTION \n "); 
            // stop_advertise(); 
            stop_pairing_ble(); 
        }
        if (strcmp(tx, "wifi_list") == 0)
        {
            char *wifi_scan = wifi_list();
            // if (wifi_scan == NULL){
            //     wifi_scan =" NO Available wifi " ;
            // }
            ESP_LOGI(TAG_BLE, "concatinated string : %s ", wifi_scan);
            // os_mbuf_append(ctxt->om, "[{\"ssid\":\"TECHSASOFT 3rd\",\"rssi\": \"-75\",\"auth_mode\":\"3\"},{\"ssid\":\"TECHSASOFT roof top\",\"rssi\": \"-79\",\"auth_mode\":\"3\"},{\"ssid\":\"TECHSASOFT\",\"rssi\": \"-87\",\"auth_mode\":\"3\"},{\"ssid\":\"TECHSASOFT 2F\",\"rssi\": \"-88\",\"auth_mode\":\"3}]", strlen("[{\"ssid\":\"TECHSASOFT 3rd\",\"rssi\": \"-75\",\"auth_mode\":\"3\"},{\"ssid\":\"TECHSASOFT roof top\",\"rssi\": \"-79\",\"auth_mode\":\"3\"},{\"ssid\":\"TECHSASOFT\",\"rssi\": \"-87\",\"auth_mode\":\"3\"},{\"ssid\":\"TECHSASOFT 2F\",\"rssi\": \"-88\",\"auth_mode\":\"3}]"));
            os_mbuf_append(ctxt->om, wifi_scan, strlen(wifi_scan));
            wifi_scan = NULL;
            free(wifi_scan);
        }
        else if (strcmp(tx, "device_id") == 0)
        {
            get_mac_address();
            os_mbuf_append(ctxt->om, device_uuid, strlen(device_uuid));
        }
        else if (strcmp(tx, "credentials") == 0)
        {

            while ((device_registered != true) && retry < 3)
            {
                // create_device_aws_request();
                vTaskDelay(200 / portTICK_PERIOD_MS);
                // stay here untill 3 tries
                }
            if (status_code != 404)
            {
                retry = 0;
                int size = snprintf(NULL, 0, "{\"device_id\": \"%s\",\"status\" : %d }", device_uuid, status_code);
                char post_data[size];
                // sprintf(post_data,"%c",'\0');
                sprintf(post_data, "{\"device_id\": \"%s\",\"status\" : %d }", device_uuid, status_code);
                ESP_LOGI(TAG_BLE, "data to send over ble : %s ", post_data);
                os_mbuf_append(ctxt->om, post_data, sizeof(post_data));
                pairing_completed = true;
            }
            else
            {
                retry = 0;
                char post_data[35] = {'\0'};
                sprintf(post_data, "{\"device_id\": \"%s\",\"status\" : %d}", " ", status_code);
                ESP_LOGI(TAG_BLE, "data to send over ble : %s ", post_data);
                os_mbuf_append(ctxt->om, post_data, sizeof(post_data));
            }
            // stop_pairing_ble(); 
        }
        else
        {

            os_mbuf_append(ctxt->om, "wrong input", strlen("wrong input"));
        }
    }
    else
    {
        os_mbuf_append(ctxt->om, "no DATA", strlen("no DATA"));
    }

    return 0;
}

// Array of pointers to other service definitions
// UUID - Universal Unique Identifier
static const struct ble_gatt_svc_def gatt_svcs[] = {
    {.type = BLE_GATT_SVC_TYPE_PRIMARY,
     .uuid = BLE_UUID16_DECLARE(0x180), // Define UUID for device type
     .characteristics = (struct ble_gatt_chr_def[]){
         {.uuid = BLE_UUID16_DECLARE(0xFEF4), // Define UUID for reading
          .flags = BLE_GATT_CHR_F_READ,
          .access_cb = device_read},
         {.uuid = BLE_UUID16_DECLARE(0xDEAD), // Define UUID for writing
          .flags = BLE_GATT_CHR_F_WRITE,
          .access_cb = device_write},
         {0}}},
    {0}};

// BLE event handling
static int ble_gap_event(struct ble_gap_event *event, void *arg)
{
    switch (event->type)
    {
    // Advertise if connected
    case BLE_GAP_EVENT_CONNECT:
        ESP_LOGI("GAP", "BLE GAP EVENT CONNECT %s", event->connect.status == 0 ? "OK!" : "FAILED!");
        if (event->connect.status != 0)
        {
            ble_app_advertise();
        }
        else
        {
            device_connected = true ; 
#ifdef wifi_ble_both
            stop_webserver();
#endif
        }
        break;
    case BLE_GAP_EVENT_DISCONNECT:
        ESP_LOGI(TAG_BLE, "device disconnected\n\n");
        ESP_LOGI(TAG_BLE, "disconnect; reason=%d ", event->disconnect.reason);
        if (pairing_completed == true)
        {
            ble_gap_adv_stop();
        }
        else
        {
            ble_app_advertise();
        }
        break;
    // Advertise again after completion of the event
    case BLE_GAP_EVENT_ADV_COMPLETE:
        ESP_LOGI("GAP", "BLE GAP EVENT");
        ble_app_advertise();
        break;
    default:
        break;
    }
    return 0;
}

// Define the BLE connection
void ble_app_advertise(void)
{
    // NimBLEAdvertisementData adv_data ;
    //  GAP - device name definition
    struct ble_hs_adv_fields fields;
    // ESP_LOGE(TAG_BLE,"size of fields : %d \n\n", sizeof(fields));
    // uint8_t man_arr[] = { 0x31, 0x31, 0x32, 0x31, 0x30, 0x30};
    const char *device_name;
    const char *manuf_data = "a11299";
    memset(&fields, 0, sizeof(fields));
    device_name = ble_svc_gap_device_name(); // Read the BLE device name
    fields.name = (uint8_t *)device_name;
    fields.name_len = strlen(device_name);
    // ESP_LOGE(TAG_BLE, "size of Device name : %d \n\n", strlen(device_name));
    fields.name_is_complete = 1;
    fields.mfg_data = (uint8_t *)manuf_data;
    fields.mfg_data_len = strlen(manuf_data);
    int err = ble_gap_adv_set_fields(&fields);
    if (err == BLE_HS_EMSGSIZE)
    {
        ESP_LOGE(TAG_BLE, "Big advertising data : %d\n\n", strlen(manuf_data));
    }
    // GAP - device connectivity definition
    struct ble_gap_adv_params adv_params;
    memset(&adv_params, 0, sizeof(adv_params));
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND; // connectable or non-connectable
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN; // discoverable or non-discoverable
    // adv_data.setManufacturerData("this is advertising data");
    ble_gap_adv_start(ble_addr_type, NULL, BLE_HS_FOREVER, &adv_params, ble_gap_event, NULL);
}

// The application
void ble_app_on_sync(void)
{
    ble_hs_id_infer_auto(0, &ble_addr_type); // Determines the best address type automatically
    ble_app_advertise();                     // Define the BLE connection
}

// The infinite task
void host_task(void *param)
{
    nimble_port_run(); // This function will return only when nimble_port_stop() is executed
    nimble_port_freertos_deinit();
}

bool start_pairing_ble()
{
#ifdef only_ble
    wifi_init_sta_ap();
#endif
    //........................
    esp_nimble_hci_and_controller_init();               // 2 - Initialize ESP controller
    nimble_port_init();                                 // 3 - Initialize the host stack
    ble_svc_gap_device_name_set(DEVICE_ADVERTISE_NAME); // 4 - Initialize NimBLE configuration - server name
    ble_svc_gap_init();                                 // 4 - Initialize NimBLE configuration - gap service
    ble_svc_gatt_init();                                // 4 - Initialize NimBLE configuration - gatt service
    ble_gatts_count_cfg(gatt_svcs);                     // 4 - Initialize NimBLE configuration - config gatt services

    ble_gatts_add_svcs(gatt_svcs);        // 4 - Initialize NimBLE configuration - queues gatt services.
    ble_hs_cfg.sync_cb = ble_app_on_sync; // 5 - Initialize application
    nimble_port_freertos_init(host_task); // 6 - Run the thread
    char txBuffer[70];
    queue = xQueueCreate(1, sizeof(txBuffer));
    return true;
}

int stop_advertise(void)
{

    return ble_gap_adv_stop();
}
bool stop_pairing_ble(void)
{
    int ret = nimble_port_stop();
    if (ret == 0)
    {
        nimble_port_deinit();

        ret = esp_nimble_hci_and_controller_deinit();
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG_BLE, "esp_nimble_hci_and_controller_deinit() failed with error: %d", ret);
            return false;
        }
        else
        {
            return true;
        }
    }
    else
    {
        return false;
    }
}