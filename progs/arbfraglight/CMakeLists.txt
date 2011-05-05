set(SOURCES
	arbfraglight.c
)
add_executable(arbfraglight ${SOURCES})
target_link_libraries(arbfraglight crutil crfaker crmothership spuload)
