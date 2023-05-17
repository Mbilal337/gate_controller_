set(AWS_INCLUDE_DIRS
	"."
    "${CMAKE_CURRENT_LIST_DIR}/../esp-aws-iot/libraries/common/logging"
    "${CMAKE_CURRENT_LIST_DIR}/../esp-aws-iot/libraries/coreMQTT"
    "${CMAKE_CURRENT_LIST_DIR}/../esp-aws-iot/libraries/coreMQTT-agent"
    "${CMAKE_CURRENT_LIST_DIR}/../esp-aws-iot/libraries/corePKCS11"
    "${CMAKE_CURRENT_LIST_DIR}/../esp-aws-iot/libraries/Device-shadow-for-AWS-IoT-embedded-sdk"
    "${CMAKE_CURRENT_LIST_DIR}/../esp-aws-iot/libraries/backoffAlgorithm"
    "${CMAKE_CURRENT_LIST_DIR}/../esp-aws-iot/libraries/common/posix_compat"
)

idf_component_register(
		SRCS
			"aws_flow.c" 
		INCLUDE_DIRS
		${AWS_INCLUDE_DIRS}
	)

set(COMPONENT_ADD_INCLUDEDIRS "." )

target_add_binary_data(${COMPONENT_TARGET} "../../main/certs/root_cert_auth.crt" TEXT)
target_add_binary_data(${COMPONENT_TARGET} "../../main/certs/client.crt" TEXT)
target_add_binary_data(${COMPONENT_TARGET} "../../main/certs/client.key" TEXT) 