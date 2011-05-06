include_directories(.)
set(SOURCES
	motionblurspu.c
	motionblurspu_config.c
	motionblurspu_init.c
)
add_library(motionblurspu SHARED ${SOURCES})
target_link_libraries(motionblurspu spuload crutil crmothership)
target_link_libraries(motionblurspu ${EXTRA_LIBS})
install(TARGETS motionblurspu
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)
