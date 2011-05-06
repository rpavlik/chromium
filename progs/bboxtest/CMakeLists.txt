include_directories(.)
set(SOURCES
	bboxtest.c
)
add_executable(bboxtest  ${SOURCES})
target_link_libraries(bboxtest ${EXTRA_LIBS})
