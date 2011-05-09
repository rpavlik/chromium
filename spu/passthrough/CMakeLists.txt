include_directories(.)

set(SOURCES
	passthroughspu_init.c
	${CMAKE_CURRENT_BINARY_DIR}/passthroughspu.c
	passthrough.def
)
generate_with_python(passthrough.py passthroughspu.c ${APIFILES})

add_library(passthroughspu SHARED ${SOURCES})
target_link_libraries(passthroughspu crutil)
target_link_libraries(passthroughspu ${EXTRA_LIBS})
install(TARGETS passthroughspu
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)

# header dependency
add_dependencies(crstate spuload)
