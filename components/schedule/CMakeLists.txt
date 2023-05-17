idf_component_register(SRCS "Schedule.c"
                       INCLUDE_DIRS "."
                       REQUIRES "common" "nvs" "spiffs" "json")