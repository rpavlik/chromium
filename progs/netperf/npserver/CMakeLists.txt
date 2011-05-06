include_directories(.)
set(SOURCES
	npserver.c
)
add_executable(npserver  ${SOURCES})
target_link_libraries(npserver crmothership crutil)
target_link_libraries(npserver ${EXTRA_LIBS})
install(TARGETS npserver
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)
