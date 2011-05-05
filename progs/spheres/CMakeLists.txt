set(SOURCES
	spheres.c
)
add_executable(spheres ${SOURCES})
target_link_libraries(spheres crutil crfaker crmothership spuload)
