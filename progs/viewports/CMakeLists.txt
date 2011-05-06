include_directories(.)
set(SOURCES
	viewports.c
)
add_executable(viewports  ${SOURCES})
target_link_libraries(viewports ${EXTRA_LIBS})
install(TARGETS viewports
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)
