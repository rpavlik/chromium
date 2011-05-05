set(SOURCES
	app_faker.c
)
add_executable(crappfaker ${SOURCES})
target_link_libraries(crappfaker crmothership crutil)
