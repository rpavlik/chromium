include_directories(.)
set(SOURCES
	arrayspu.c
	arrayspu_config.c
	arrayspu_init.c
)
add_library(arrayspu SHARED ${SOURCES})
target_link_libraries(arrayspu spuload crutil crmothership crstate)
target_link_libraries(arrayspu ${EXTRA_LIBS})
