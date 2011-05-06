include_directories(.)
set(SOURCES
	dmx.c
)
add_library(dmxutil SHARED ${SOURCES})
target_link_libraries(dmxutil ${EXTRA_LIBS})
