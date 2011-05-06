include_directories(.)
set(SOURCES
	stereocube.c
)
add_executable(stereocube  ${SOURCES})
target_link_libraries(stereocube ${EXTRA_LIBS})
install(TARGETS stereocube
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)
