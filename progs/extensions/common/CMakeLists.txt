include_directories(.)
set(SOURCES
	logo.c
	util.c
)
add_library(crextensionscommon SHARED ${SOURCES})
target_link_libraries(crextensionscommon crutil)
target_link_libraries(crextensionscommon ${EXTRA_LIBS})
install(TARGETS crextensionscommon
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)
