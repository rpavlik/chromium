include_directories(.)
set(SOURCES
	reset.c
)
add_executable(resetms  ${SOURCES})
target_link_libraries(resetms crmothership crutil)
target_link_libraries(resetms ${EXTRA_LIBS})
install(TARGETS resetms
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)
