set(SOURCES
	mattespu.c
	mattespu_config.c
	mattespu_init.c
)
add_library(mattespu SHARED ${SOURCES})
target_link_libraries(mattespu spuload crutil crmothership)
