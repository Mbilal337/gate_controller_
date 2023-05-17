#include "pairing_wifi.h"
#include "common.h"
#ifdef wifi_ble_both
#include "pairing_ble.h"
#endif
// / WIFI AP+STA
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_http_client.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include <sys/param.h>
#include "esp_netif.h"
#include <esp_http_server.h>
//.................................................................................
#include <stdio.h>
#include "freertos/event_groups.h"
#include "cJSON.h"

//............
static const char *TAG_WIFI = "Pairing_Mode_WIFI";

int retry = 0;

#define DEFAULT_SCAN_LIST_SIZE 10
static httpd_handle_t server_1 = NULL;
wifi_config_t wifi_config_st;
wifi_config_t wifi_config_ap;
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

#define STA_EXAMPLE_ESP_WIFI_SSID "TECHSASOFT 3rd"
#define STA_EXAMPLE_ESP_WIFI_PASS "0000151"
#define STA_EXAMPLE_ESP_MAXIMUM_RETRY 3

static EventGroupHandle_t s_wifi_event_group;

#define DEVICE_ADVERTISE_NAME "Gate_Controller_a1112"
#define DEVICE_ADVERTISE_PASS "mypassword"
#define EXAMPLE_ESP_WIFI_CHANNEL 1
#define EXAMPLE_MAX_STA_CONN 4

static void create_device_aws_request();
void wifi_init_sta_ap(void);
bool sta_wifi_disconnect_event = false;


static void print_auth_mode(int authmode)
{
    switch (authmode)
    {
    case WIFI_AUTH_OPEN:
        ESP_LOGI(TAG_WIFI, "Authmode \tWIFI_AUTH_OPEN");
        break;
    case WIFI_AUTH_WEP:
        ESP_LOGI(TAG_WIFI, "Authmode \tWIFI_AUTH_WEP");
        break;
    case WIFI_AUTH_WPA_PSK:
        ESP_LOGI(TAG_WIFI, "Authmode \tWIFI_AUTH_WPA_PSK");
        break;
    case WIFI_AUTH_WPA2_PSK:
        ESP_LOGI(TAG_WIFI, "Authmode \tWIFI_AUTH_WPA2_PSK");
        break;
    case WIFI_AUTH_WPA_WPA2_PSK:
        ESP_LOGI(TAG_WIFI, "Authmode \tWIFI_AUTH_WPA_WPA2_PSK");
        break;
    case WIFI_AUTH_WPA2_ENTERPRISE:
        ESP_LOGI(TAG_WIFI, "Authmode \tWIFI_AUTH_WPA2_ENTERPRISE");
        break;
    case WIFI_AUTH_WPA3_PSK:
        ESP_LOGI(TAG_WIFI, "Authmode \tWIFI_AUTH_WPA3_PSK");
        break;
    case WIFI_AUTH_WPA2_WPA3_PSK:
        ESP_LOGI(TAG_WIFI, "Authmode \tWIFI_AUTH_WPA2_WPA3_PSK");
        break;
    default:
        ESP_LOGI(TAG_WIFI, "Authmode \tWIFI_AUTH_UNKNOWN");
        break;
    }
}

char device_uuid[30]; // MAC ADDRESS without colons

/// @brief
//          Get device Mac Address
/// @param
//      None
/// @return
//      None
void get_mac_address()
{
    uint8_t base_mac_addr[6];
    esp_err_t ret = ESP_OK;
    // Get base MAC address from EFUSE BLK0(default option)
    ret = esp_efuse_mac_get_default(base_mac_addr);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG_WIFI, "Failed to get base MAC address from EFUSE BLK0. (%s)", esp_err_to_name(ret));
        ESP_LOGE(TAG_WIFI, "Aborting");
        abort();
    }
    else
    {
        ESP_LOGI(TAG_WIFI, "Base MAC Address read from EFUSE BLK0");
        // Set the base MAC address using the retrieved MAC address
        sprintf(device_uuid, "%x%x%x%x%x%x", base_mac_addr[0], base_mac_addr[1], base_mac_addr[2], base_mac_addr[3], base_mac_addr[4], base_mac_addr[5]);
        device_id_acquired = true;
        // printf("Mac: %s\n", device_uuid);
    }
}

void delchar(char *string, int starting_pos, int no_of_char)
{
    if ((starting_pos + no_of_char - 1) <= strlen(string))
    {
        strcpy(&string[starting_pos + 1], &string[starting_pos + no_of_char + 1]);
        string[starting_pos] = ' ';
        printf(" ssid is %s \n ", string);
        puts(string);
    }
}
void check_wifi_name(char *ssid)
{
    printf(" ssid is %s \n ", ssid);
    for (int i = 0; i < strlen(ssid); i++)
    {

        if (ssid[i] == '%')
        {
            printf("at charachter %d \n", i);

            if (ssid[i + 1] == '2')
            {

                if (ssid[i + 2] == '0')
                {
                    ssid[i + 2] = ' ';
                    printf(" ssid is %s \n ", ssid);
                    delchar(ssid, i, 2);
                }
                else
                {
                    printf("no consective ");
                }
            }
            else
            {
                printf("only1");
            }
        }
    }
    printf("final string is %s \n\n", ssid);
}

static int s_retry_num = 0;
esp_err_t client_event_post_handler(esp_http_client_event_t *evt)
{
    static char *output_buffer; // Buffer to store response of http request from event handler
    static int output_len;      // Stores number of bytes read
    switch (evt->event_id)
    {
    case HTTP_EVENT_ERROR:
        ESP_LOGD(TAG_WIFI, "HTTP_EVENT_ERROR");
        retry++;
        break;
    case HTTP_EVENT_ON_CONNECTED:
        ESP_LOGD(TAG_WIFI, "HTTP_EVENT_ON_CONNECTED");
        break;
    case HTTP_EVENT_HEADER_SENT:
        ESP_LOGD(TAG_WIFI, "HTTP_EVENT_HEADER_SENT");
        break;
    case HTTP_EVENT_ON_HEADER:
        ESP_LOGD(TAG_WIFI, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
        break;
    case HTTP_EVENT_ON_DATA:
        ESP_LOGD(TAG_WIFI, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
        /*
         *  Check for chunked encoding is added as the URL for chunked encoding used in this example returns binary data.
         *  However, event handler can also be used in case chunked encoding is used.
         */
        if (!esp_http_client_is_chunked_response(evt->client))
        {
            // If user_data buffer is configured, copy the response into the buffer
            if (evt->user_data)
            {
                memcpy(evt->user_data + output_len, evt->data, evt->data_len);
            }
            else
            {
                if (output_buffer == NULL)
                {
                    output_buffer = (char *)malloc(esp_http_client_get_content_length(evt->client));
                    output_len = 0;
                    if (output_buffer == NULL)
                    {
                        ESP_LOGE(TAG_WIFI, "Failed to allocate memory for output buffer");
                        return ESP_FAIL;
                    }
                }
                memcpy(output_buffer + output_len, evt->data, evt->data_len);
            }
            output_len += evt->data_len;
        }

        break;
    case HTTP_EVENT_ON_FINISH:

        ESP_LOGD(TAG_WIFI, "HTTP_EVENT_ON_FINISH data recieved is : %s \n", output_buffer);
        cJSON *root = cJSON_Parse(output_buffer);
        int root_array_size = cJSON_GetArraySize(root);
        ESP_LOGI(TAG_WIFI, "items in cJSON are %d ", root_array_size);
        cJSON *element = cJSON_GetArrayItem(root, 0);
        if (element->type == cJSON_Number)
        {
            status_code = element->valueint;
            ESP_LOGI(TAG_WIFI, "Statuscode= %d", element->valueint);
            device_registered = true;
            retry = 4;
        }

        cJSON_Delete(root);
        if (output_buffer != NULL)
        {
            // Response is accumulated in output_buffer. Uncomment the below line to print the accumulated response
            // ESP_LOG_BUFFER_HEX(TAG_WIFI, output_buffer, output_len);
            free(output_buffer);
            output_buffer = NULL;
        }
        output_len = 0;
        break;
    case HTTP_EVENT_DISCONNECTED:
        ESP_LOGI(TAG_WIFI, "HTTP_EVENT_DISCONNECTED");
        if (output_buffer != NULL)
        {
            free(output_buffer);
            output_buffer = NULL;
        }
        output_len = 0;
        retry = 4;
        break;
    }
    return ESP_OK;
}

/// @brief
//          Create device on aws
/// @param
//      None
/// @return
//      None
static void create_device_aws_request()
{
    esp_http_client_config_t config_post = {
        .url = "https://87sz0mfyuh.execute-api.us-east-2.amazonaws.com/v1/device",
        .method = HTTP_METHOD_POST,
        .cert_pem = NULL,
        .event_handler = client_event_post_handler};

    esp_http_client_handle_t client = esp_http_client_init(&config_post);
    char post_data[75];
    if (device_id_acquired != true)
    {
        get_mac_address();
    }
    sprintf(post_data, "{\"device_uuid\": \"%s\" , \"device_type_id\" : 10}", device_uuid);
    esp_http_client_set_post_field(client, post_data, strlen(post_data));
    esp_http_client_set_header(client, "Content-Type", "application/json");
    if (wifi_connected == true)
    {

        esp_http_client_perform(client);
        esp_http_client_cleanup(client);
    }
    else
    {
        device_registered = false;
        retry = 4;
    }
}

/// @brief
//          Scan and get Wifi list
/// @param
//      None
/// @return
//      Char * -> Wifi list (in json format formated)
char *wifi_list()
{
    //----------------------------
    // if (s_retry_num < 3 && wifi_connected == false)
    // {
    //     ESP_LOGI(TAG_WIFI,"disconnecting and restarting again");
    //     ESP_ERROR_CHECK(esp_wifi_stop());
    //     ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    //     ESP_ERROR_CHECK(esp_wifi_start());
    // }
#ifdef only_ble
    // wifi_init_sta_ap();
    // // ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(TAG_WIFI, "ONLY BLE CASE\n");
    // ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
#endif
    uint16_t number = DEFAULT_SCAN_LIST_SIZE;
    wifi_ap_record_t ap_info[DEFAULT_SCAN_LIST_SIZE];
    uint16_t ap_count = 0;
    memset(ap_info, 0, sizeof(ap_info));
    esp_wifi_scan_start(NULL, true);
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
    ESP_LOGI(TAG_WIFI, "Total APs scanned = %u", ap_count);
    char *resp;
    if (ap_count > 0)
    {
        char *ssid_[ap_count];
        int8_t rssi[ap_count];
        uint8_t authmode[ap_count];
        char response[65 * ap_count];
        resp = (char *)malloc(sizeof(response));
        sprintf(resp, "%c", '\0');
        strcat(resp, "[");

        for (int i = 0; (i < DEFAULT_SCAN_LIST_SIZE) && (i < ap_count); i++)
        {
            // ESP_LOGI(TAG_WIFI, "SSID \t\t%s", ap_info[i].ssid);
            ssid_[i] = (char *)ap_info[i].ssid;
            // ESP_LOGI(TAG_WIFI, "RSSI \t\t%d", ap_info[i].rssi);
            rssi[i] = ap_info[i].rssi;
            print_auth_mode(ap_info[i].authmode);
            authmode[i] = ap_info[i].authmode;
            // ESP_LOGI(TAG_WIFI, "Channel \t\t%d\n", ap_info[i].primary);

            // creating response to send
            strcat(resp, "{\"ssid\":\"");
            strcat(resp, ssid_[i]);
            strcat(resp, "\",\"rssi\": ");
            char rss[5];
            sprintf(rss, "%d", rssi[i]);
            strcat(resp, rss);
            strcat(resp, ",\"auth_mode\":\"");
            sprintf(rss, "%d", authmode[i]);
            strcat(resp, rss);
            if (i < ap_count - 1)
            {
                strcat(resp, "\"},");
            }
        }
        strcat(resp, "\"}]");

        // ESP_LOGI(TAG_WIFI, "Concatenated in string is \n\n : %s \n\n", resp);

        //--------------------
    }
    else
    {
        // char response[]="NO Available wifi" ;
        // resp = (char *)malloc(sizeof(response));
        resp = "NO Available wifi";
    }

    return resp;
}

// /* An HTTP GET handler */
static esp_err_t credentials_get_handler(httpd_req_t *req)
{
    char *buf;
    size_t buf_len;
#ifdef DEBUG_ON
    ESP_LOGI(TAG_WIFI, "Here///////////////////////");
#endif
    buf_len = httpd_req_get_hdr_value_len(req, "Host") + 1;
    if (buf_len > 1)
    {
        buf = malloc(buf_len);
        /* Copy null terminated value string into buffer */
        if (httpd_req_get_hdr_value_str(req, "Host", buf, buf_len) == ESP_OK)
        {
            ESP_LOGI(TAG_WIFI, "Found header => Host: %s", buf);
        }
        free(buf);
    }

    buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1)
    {
        buf = malloc(buf_len);
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK)
        {
            ESP_LOGI(TAG_WIFI, "Found URL query => %s", buf);
            /* Get value of expected key from query string */
            if (httpd_query_key_value(buf, "ssid", wifi_credentials.param_SSID, sizeof(wifi_credentials.param_SSID)) == ESP_OK)
            {
                ESP_LOGI(TAG_WIFI, "Received query parameter => ssid=%s", wifi_credentials.param_SSID);
                check_wifi_name(wifi_credentials.param_SSID);
                strcpy((char *)wifi_config_st.sta.ssid, wifi_credentials.param_SSID);
                ESP_LOGI(TAG_WIFI, "Received query parameter => ssid=%s", (char *)wifi_config_st.sta.ssid);
            }

            if (httpd_query_key_value(buf, "offset", wifi_credentials.offset, sizeof(wifi_credentials.offset)) == ESP_OK)
            {
                ESP_LOGI(TAG_WIFI, "Received query parameter => offset=%s", wifi_credentials.offset);
            }

            if (httpd_query_key_value(buf, "password", wifi_credentials.param_Password, sizeof(wifi_credentials.param_Password)) == ESP_OK)
            {

                strcpy((char *)wifi_config_st.sta.password, wifi_credentials.param_Password);
                ESP_LOGI(TAG_WIFI, "Received query parameter => password=%s", (char *)wifi_config_st.sta.password);
                credential_recieved = true;
            }
            if (httpd_query_key_value(buf, "region", wifi_credentials.region, sizeof(wifi_credentials.region)) == ESP_OK)
            {
                ESP_LOGI(TAG_WIFI, "Received query parameter => region=%s", wifi_credentials.region);
            }
        }
        if (credential_recieved == true)
        {
            ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config_st));
            // esp_wifi_connect();
            s_retry_num = 0;
            // ESP_ERROR_CHECK(esp_wifi_stop());
            // ESP_ERROR_CHECK(esp_wifi_start());
            ESP_ERROR_CHECK(esp_wifi_connect());
        }
        free(buf);
    }
    while ((device_registered != true) && (retry < 3))
    {
        ESP_LOGI(TAG_WIFI, "waiting to register device");
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
    if (status_code != 404)
    {
        retry = 0;
        int size = snprintf(NULL, 0, "{\"device_id\": \"%s\",\"status\" : %d }", device_uuid, status_code);
        char post_data[size];
        // sprintf(post_data,"%c",'\0');
        sprintf(post_data, "{\"device_id\": \"%s\",\"status\" : %d }", device_uuid, status_code);
        ESP_LOGI(TAG_WIFI, "data to send over webserver : %s ", post_data);
        esp_err_t error = httpd_resp_send(req, post_data, strlen(post_data));
        if (error != ESP_OK)
        {
            ESP_LOGI(TAG_WIFI, "Error %d while sending Response", error);
        }
        else
        {
            ESP_LOGI(TAG_WIFI, "Response sent successfully");
        }
        if (httpd_req_get_hdr_value_len(req, "Host") == 0)
        {
            ESP_LOGI(TAG_WIFI, "Request headers lost");
        }
        return ESP_OK;
    }
    else
    {
        retry = 0;
        char post_data[35] = {'\0'};
        sprintf(post_data, "{\"device_id\": \"%s\",\"status\" : %d}", " ", status_code);
        ESP_LOGI(TAG_WIFI, "data to send over webserver: %s ", post_data);
        esp_err_t error = httpd_resp_send(req, post_data, strlen(post_data));
        if (error != ESP_OK)
        {
            ESP_LOGI(TAG_WIFI, "Error %d while sending Response", error);
        }
        else
        {
            ESP_LOGI(TAG_WIFI, "Response sent successfully");
        }
        if (httpd_req_get_hdr_value_len(req, "Host") == 0)
        {
            ESP_LOGI(TAG_WIFI, "Request headers lost");
        }
        return ESP_OK;
    }
    // printf("Length: %d\n", length);
}

static const httpd_uri_t ssid_AP = {
    .uri = "/ssid",
    .method = HTTP_GET,
    .handler = credentials_get_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx = "Got SSID!"};

static esp_err_t wifi_list_handler(httpd_req_t *req)
{
    //----------------------------
    char *resp = wifi_list();
    ESP_LOGI(TAG_WIFI, "Concatenated in string is \n\n : %s \n\n", resp);

    //--------------------
    esp_err_t err;
    // ESP_LOGI(TAG_WIFI, "WIFI SCANED");
    const char *response = (const char *)resp;
    err = httpd_resp_send(req, response, strlen(response));
    if (err != ESP_OK)
    {
        ESP_LOGI(TAG_WIFI, "ERROR %d while sending response ", err);
    }
    else
        ESP_LOGI(TAG_WIFI, "Response sent");

    if (httpd_req_get_hdr_value_len(req, "Host") == 0)
    {
        ESP_LOGI(TAG_WIFI, "Request headers lost");
    }
    resp = NULL;
    free(resp);
    return err;
}

static const httpd_uri_t scanwifi = {
    .uri = "/wifi_list",
    .method = HTTP_GET,
    .handler = wifi_list_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx = "0"};

esp_err_t http_404_error_handler(httpd_req_t *req, httpd_err_code_t err)
{
    /* For any other URI send 404 and close socket */
    httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Some 404 error message");
    return ESP_FAIL;
}

static httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;

    // Start the httpd server
    ESP_LOGI(TAG_WIFI, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK)
    {
        // Set URI handlers
        ESP_LOGI(TAG_WIFI, "Registering URI handlers");
        httpd_register_uri_handler(server, &scanwifi);
        httpd_register_uri_handler(server, &ssid_AP);
        return server;
    }

    ESP_LOGI(TAG_WIFI, "Error starting server!");
    return NULL;
}

void stop_webserver()
{
    // Stop the httpd server
    httpd_stop(server_1);
}

static void connect_handler(void *arg, esp_event_base_t event_base,
                            int32_t event_id, void *event_data)
{
    device_connected = true ; 
// Stop BLE (
#ifdef wifi_ble_both

    int status = stop_advertise();
    if (status == 0)
    {
        ESP_LOGI(TAG_WIFI, "BLE Advertising stoped please check \n\n\n ");
    }
#endif
    // Start webServer
    server_1 = (httpd_handle_t)&arg;
    httpd_handle_t *server = (httpd_handle_t *)arg;
    if (*server == NULL)
    {
        ESP_LOGI(TAG_WIFI, "Starting webserver");
        *server = start_webserver();
    }
}

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        ESP_LOGI(TAG_WIFI, "Starting wifi station \n\n");
        sta_wifi_disconnect_event = true;
        // esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (s_retry_num < 3)
        {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG_WIFI, "retry to connect to the AP");
        }
        else
        {
            ESP_LOGI(TAG_WIFI, "connect to the AP fail");
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        sta_wifi_disconnect_event = true;
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG_WIFI, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        sta_wifi_disconnect_event = false;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        wifi_connected = true;
        while ((device_registered == false) && (retry < 3))
        {
            retry++;
            create_device_aws_request();
        }
    }
    if (event_id == WIFI_EVENT_STA_CONNECTED)
    {
        ESP_LOGI(TAG_WIFI, "sta connected\n\n");
    }
    if (event_id == WIFI_EVENT_AP_START)
    {
        ESP_LOGI(TAG_WIFI, "AP Started \n\n");
    }
    if (event_id == WIFI_EVENT_AP_STACONNECTED)
    {
        wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *)event_data;
        ESP_LOGI(TAG_WIFI, "station " MACSTR " join, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
    else if (event_id == WIFI_EVENT_AP_STADISCONNECTED)
    {
        wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *)event_data;
        ESP_LOGI(TAG_WIFI, "station " MACSTR " leave, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
}
void wifi_mode_sta_only()
{
    ESP_LOGI(TAG_WIFI, "ESP_WIFI_MODE_STA_ONLY");
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config_st));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_connect());
}

void wifi_mode_ap_only()
{
    ESP_LOGI(TAG_WIFI, "ESP_WIFI_MODE_AP_ONLY");
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config_ap));
    ESP_LOGI(TAG_WIFI, "wifi_init_softap finished. SSID:%s password:%s channel:%d",
             DEVICE_ADVERTISE_NAME, DEVICE_ADVERTISE_PASS, EXAMPLE_ESP_WIFI_CHANNEL);
    ESP_ERROR_CHECK(esp_wifi_start());
}

void wifi_init_sta_ap(void)
{
    s_wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    esp_netif_create_default_wifi_ap();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config_1 = {
        .sta = {
            .ssid = STA_EXAMPLE_ESP_WIFI_SSID,
            .password = STA_EXAMPLE_ESP_WIFI_PASS,
            /* Setting a password implies station will connect to all security modes including WEP/WPA.
             * However these modes are deprecated and not advisable to be used. Incase your Access point
             * doesn't support WPA2, these mode can be enabled by commenting below line */
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
        },
    };
    wifi_config_st = wifi_config_1;

    wifi_config_t wifi_config_2 = {
        .ap = {
            .ssid = DEVICE_ADVERTISE_NAME,
            .ssid_len = strlen(DEVICE_ADVERTISE_NAME),
            .channel = EXAMPLE_ESP_WIFI_CHANNEL,
            .password = DEVICE_ADVERTISE_PASS,
            .max_connection = EXAMPLE_MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK},
    };
    if (strlen(DEVICE_ADVERTISE_PASS) == 0)
    {
        wifi_config_2.ap.authmode = WIFI_AUTH_OPEN;
    }
    wifi_config_ap = wifi_config_2;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config_st));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config_ap));

    ESP_LOGI(TAG_WIFI, "wifi_init_sta finished.");
}

static void disconnect_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    httpd_handle_t *server = (httpd_handle_t *)arg;
    if (*server)
    {
        server_1 = &server;
        ESP_LOGI(TAG_WIFI, "Stopping webserver");
        stop_webserver();
        server_1 = NULL;
    }
    if (pairing_completed == true)
    {
        wifi_mode_sta_only();
    }
    else
    {
#ifdef wifi_ble_both

        ble_app_advertise();
        ESP_LOGI(TAG_WIFI, "Starting BLE advertising check \n\n\n\n");
#endif
    }
}
bool start_pairing_wifi()
{
    static httpd_handle_t server = NULL;
    wifi_init_sta_ap();
    ESP_ERROR_CHECK(esp_wifi_start());
    // wifi_mode_ap_only();
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_AP_STAIPASSIGNED, &connect_handler, &server));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_AP_STADISCONNECTED, &disconnect_handler, &server));
    return true;
}

bool check_wifi_connected()
{
    if (sta_wifi_disconnect_event == true)
        return false;
    else
        return true;
}