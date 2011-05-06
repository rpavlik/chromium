include_directories(.)
set(SOURCES
	grabberspu.c
	grabberspu_config.c
	grabberspu_init.c
)
add_library(grabberspu SHARED ${SOURCES})
target_link_libraries(grabberspu crutil crmothership spuload)
target_link_libraries(grabberspu ${EXTRA_LIBS})
