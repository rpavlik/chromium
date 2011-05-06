include_directories(.)
set(SOURCES
	app_faker.c
)
add_executable(crappfaker  ${SOURCES})
target_link_libraries(crappfaker crmothership crutil)
target_link_libraries(crappfaker ${EXTRA_LIBS})
install(TARGETS crappfaker
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)
