include_directories(.)
set(GENERATED
	expando.c
)
# TODO: generate these files!


set(SOURCES
	expandospu.c
	expandospu_config.c
	expandospu_init.c
	${CMAKE_CURRENT_BINARY_DIR}/expando.c
)
add_library(expandospu SHARED ${SOURCES})
target_link_libraries(expandospu spuload crutil crmothership crdlm crstate)
target_link_libraries(expandospu ${EXTRA_LIBS})
install(TARGETS expandospu
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)
