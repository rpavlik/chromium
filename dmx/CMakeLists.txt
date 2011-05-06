include_directories(.)
set(SOURCES
	dmx.c
)
add_library(dmxutil SHARED ${SOURCES})
target_link_libraries(dmxutil ${EXTRA_LIBS})
install(TARGETS dmxutil
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)
