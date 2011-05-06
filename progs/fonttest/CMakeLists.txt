include_directories(.)
set(SOURCES
	fonttest.c
)
add_executable(fonttest  ${SOURCES})
target_link_libraries(fonttest ${EXTRA_LIBS})
install(TARGETS fonttest
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)
