idf_component_register(SRCS "pairing.c" "pairing_ble.c" "pairing_wifi.c"
                  INCLUDE_DIRS "."  
                  REQUIRES "esp_http_server" "nvs" "bt" "esp_http_client" "json" 
                  )



#  register_component()
