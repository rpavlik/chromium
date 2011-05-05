set(SOURCES
	fraglight.c
)
add_executable(fraglight ${SOURCES})
target_link_libraries(fraglight crutil crfaker crmothership spuload)
