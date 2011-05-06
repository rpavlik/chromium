include_directories(.)
set(SOURCES
	main.c
)
add_executable(crutserver  ${SOURCES})
target_link_libraries(crutserver crmothership crutil crutapi)
target_link_libraries(crutserver ${EXTRA_LIBS})
