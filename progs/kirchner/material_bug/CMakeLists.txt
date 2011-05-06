include_directories(.)
set(SOURCES
	bug.c
)
add_executable(kirchner_material  ${SOURCES})
target_link_libraries(kirchner_material ${EXTRA_LIBS})
install(TARGETS kirchner_material
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)
