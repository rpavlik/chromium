include_directories(.)
set(GENERATED
	simplequeryspu.c
)
# TODO: generate these files!


set(SOURCES
	simplequeryspu_config.c
	simplequeryspu_init.c
	${CMAKE_CURRENT_BINARY_DIR}/simplequeryspu.c
)
add_library(simplequeryspu SHARED ${SOURCES})
target_link_libraries(simplequeryspu spuload crutil crmothership crstate)
target_link_libraries(simplequeryspu ${EXTRA_LIBS})
install(TARGETS simplequeryspu
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)
