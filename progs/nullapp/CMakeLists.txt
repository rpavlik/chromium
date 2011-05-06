include_directories(.)
set(SOURCES
	nullapp.c
)
add_executable(nullapp  ${SOURCES})
target_link_libraries(nullapp ${EXTRA_LIBS})
install(TARGETS nullapp
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)
