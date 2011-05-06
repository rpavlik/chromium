include_directories(.)
set(SOURCES
	pinchtest.c
)
add_executable(pinchtest  ${SOURCES})
target_link_libraries(pinchtest ${EXTRA_LIBS})
