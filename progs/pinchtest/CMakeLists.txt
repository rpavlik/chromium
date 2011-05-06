include_directories(.)
set(SOURCES
	pinchtest.c
)
add_executable(pinchtest  ${SOURCES})
target_link_libraries(pinchtest ${EXTRA_LIBS})
install(TARGETS pinchtest
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)
