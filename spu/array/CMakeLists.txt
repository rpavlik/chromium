include_directories(.)
set(SOURCES
	arrayspu.c
	arrayspu_config.c
	arrayspu_init.c
	arrayspu.def
)
add_library(arrayspu SHARED ${SOURCES})
target_link_libraries(arrayspu spuload crutil crmothership crstate)
target_link_libraries(arrayspu ${EXTRA_LIBS})
install(TARGETS arrayspu
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)
