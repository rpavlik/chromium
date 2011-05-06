include_directories(.)
set(SOURCES
	dlisttest.c
)
add_executable(dlisttest  ${SOURCES})
target_link_libraries(dlisttest ${EXTRA_LIBS})
install(TARGETS dlisttest
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)
