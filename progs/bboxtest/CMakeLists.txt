include_directories(.)
set(SOURCES
	bboxtest.c
)
add_executable(bboxtest  ${SOURCES})
target_link_libraries(bboxtest ${EXTRA_LIBS})
install(TARGETS bboxtest
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)
