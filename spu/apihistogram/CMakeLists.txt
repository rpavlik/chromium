include_directories(.)
set(GENERATED
	apihistogramspu.c
)
# TODO: generate these files!


set(SOURCES
	apihistogramspu_config.c
	apihistogramspu_init.c
	${CMAKE_CURRENT_BINARY_DIR}/apihistogramspu.c
	apihistogramspu.def
)

generate_with_python(histogram.py
	apihistogramspu.c
	${APIFILES})
add_library(apihistogramspu SHARED ${SOURCES})
target_link_libraries(apihistogramspu spuload crutil crmothership)
target_link_libraries(apihistogramspu ${EXTRA_LIBS})
install(TARGETS apihistogramspu
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)
