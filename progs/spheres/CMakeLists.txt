include_directories(.)
set(SOURCES
	spheres.c
)
add_executable(spheres  ${SOURCES})
target_link_libraries(spheres crutil crfaker crmothership spuload)
target_link_libraries(spheres ${EXTRA_LIBS})
install(TARGETS spheres
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)
