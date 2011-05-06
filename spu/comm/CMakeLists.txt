include_directories(.)
set(SOURCES
	commspu.c
	commspu_config.c
	commspu_init.c
)
add_library(commspu SHARED ${SOURCES})
target_link_libraries(commspu spuload crutil crmothership)
target_link_libraries(commspu ${EXTRA_LIBS})
