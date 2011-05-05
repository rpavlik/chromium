set(SOURCES
	motionblurspu.c
	motionblurspu_config.c
	motionblurspu_init.c
)
add_library(motionblurspu SHARED ${SOURCES})
target_link_libraries(motionblurspu spuload crutil crmothership)
