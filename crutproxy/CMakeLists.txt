include_directories(.)
set(SOURCES
	main.c
)
add_executable(crutproxy  ${SOURCES})
target_link_libraries(crutproxy crutil crmothership crutapi)
target_link_libraries(crutproxy ${EXTRA_LIBS})
