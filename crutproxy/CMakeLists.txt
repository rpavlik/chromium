set(SOURCES
	main.c
)
add_executable(crutproxy ${SOURCES})
target_link_libraries(crutproxy crutil crmothership crutapi)
