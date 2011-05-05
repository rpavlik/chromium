set(SOURCES
	sputest.c
)
add_executable(sputest ${SOURCES})
target_link_libraries(sputest spuload crutil crmothership)
