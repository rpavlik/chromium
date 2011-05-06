include_directories(.)
set(SOURCES
	tiletest.c
)
add_executable(tiletest  ${SOURCES})
target_link_libraries(tiletest ${EXTRA_LIBS})
install(TARGETS tiletest
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)
