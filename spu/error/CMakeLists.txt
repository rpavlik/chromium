include_directories(.)
set(GENERATED
	errorspu.c
)
# TODO: generate these files!


set(SOURCES
	errorspu_init.c
	${CMAKE_CURRENT_BINARY_DIR}/errorspu.c
)
add_library(errorspu SHARED ${SOURCES})
target_link_libraries(errorspu crutil)
target_link_libraries(errorspu ${EXTRA_LIBS})
install(TARGETS errorspu
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)
