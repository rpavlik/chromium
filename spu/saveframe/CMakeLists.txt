include_directories(.)
find_package(JPEG)
if(JPEG_FOUND)
	add_definitions(-DJPEG)
	include_directories(${JPEG_INCLUDE_DIR})
endif()

set(SOURCES
	saveframespu.c
	saveframespu_config.c
	saveframespu_init.c
	saveframespu.def
)
add_library(saveframespu SHARED ${SOURCES})
target_link_libraries(saveframespu spuload crutil crmothership)
target_link_libraries(saveframespu ${EXTRA_LIBS})
if(JPEG_FOUND)
	target_link_libraries(saveframespu ${JPEG_LIBRARIES})
endif()
install(TARGETS saveframespu
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)
