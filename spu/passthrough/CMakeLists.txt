include_directories(.)
set(GENERATED
	passthroughspu.c
)
# TODO: generate these files!


set(SOURCES
	passthroughspu_init.c
	${CMAKE_CURRENT_BINARY_DIR}/passthroughspu.c
)
add_library(passthroughspu SHARED ${SOURCES})
target_link_libraries(passthroughspu crutil)
target_link_libraries(passthroughspu ${EXTRA_LIBS})
install(TARGETS passthroughspu
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)
