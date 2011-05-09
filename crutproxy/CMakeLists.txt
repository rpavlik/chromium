include_directories(.)
set(SOURCES
	main.c
)
add_executable(crutproxy  ${SOURCES})
if(NOT WIN32)
	target_link_libraries(crutproxy crutil crmothership crutapi)
endif()
target_link_libraries(crutproxy ${EXTRA_LIBS})
install(TARGETS crutproxy
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)
