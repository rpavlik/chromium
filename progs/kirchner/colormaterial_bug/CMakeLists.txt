include_directories(.)
set(SOURCES
	bug.c
)
add_executable(kirchner_colormaterial  ${SOURCES})
target_link_libraries(kirchner_colormaterial ${EXTRA_LIBS})
install(TARGETS kirchner_colormaterial
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)
