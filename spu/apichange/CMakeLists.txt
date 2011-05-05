set(SOURCES
	apichangespu.c
	apichangespu_config.c
	apichangespu_init.c
)
add_library(apichangespu SHARED ${SOURCES})
target_link_libraries(apichangespu spuload crutil crmothership)
