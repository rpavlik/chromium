set(SOURCES
	main.c
)
add_executable(crutserver ${SOURCES})
target_link_libraries(crutserver crmothership crutil crutapi)
