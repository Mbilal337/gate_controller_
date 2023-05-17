
set(MAIN_INCLUDE_DIRS
	"."
)

idf_component_register(
		SRCS
			"main.c" "aws.c" 
		INCLUDE_DIRS
		${MAIN_INCLUDE_DIRS}
		
	)

set(COMPONENT_ADD_INCLUDEDIRS "." )

# target_add_binary_data(${COMPONENT_TARGET} "certs/root_cert_auth.crt" TEXT)
# target_add_binary_data(${COMPONENT_TARGET} "certs/client.crt" TEXT)
# target_add_binary_data(${COMPONENT_TARGET} "certs/client.key" TEXT) 
