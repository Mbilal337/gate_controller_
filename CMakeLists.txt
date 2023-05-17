# For more information about build system see
# https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html
# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
# include_directories(BEFORE "${CMAKE_CURRENT_LIST_DIR}/main")
set(EXTRA_COMPONENT_DIRS 
                "$ENV{IDF_PATH}/examples/common_components/protocol_examples_common"
                "${CMAKE_CURRENT_LIST_DIR}/components/esp-aws-iot/libraries/common/logging"
                "${CMAKE_CURRENT_LIST_DIR}/components/esp-aws-iot/libraries/coreMQTT"
                "${CMAKE_CURRENT_LIST_DIR}/components/esp-aws-iot/libraries/coreMQTT-agent"
                "${CMAKE_CURRENT_LIST_DIR}/components/esp-aws-iot/libraries/corePKCS11"
                "${CMAKE_CURRENT_LIST_DIR}/components/esp-aws-iot/libraries/Device-shadow-for-AWS-IoT-embedded-sdk"
                "${CMAKE_CURRENT_LIST_DIR}/components/esp-aws-iot/libraries/backoffAlgorithm"
                "${CMAKE_CURRENT_LIST_DIR}/components/esp-aws-iot/libraries/common/posix_compat"
                )

project(2xGateController-aws)