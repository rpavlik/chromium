include_directories(.)
set(SOURCES
	saveframespu.c
	saveframespu_config.c
	saveframespu_init.c
)
add_library(saveframespu SHARED ${SOURCES})
target_link_libraries(saveframespu spuload crutil crmothership jpeg)
target_link_libraries(saveframespu ${EXTRA_LIBS})
