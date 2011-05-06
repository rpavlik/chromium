include_directories(.)
set(SOURCES
	retile.c
)
add_executable(retile  ${SOURCES})
target_link_libraries(retile ${EXTRA_LIBS})
install(TARGETS retile
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)
