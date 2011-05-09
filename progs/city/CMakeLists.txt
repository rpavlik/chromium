include_directories(.)
set(SOURCES
	city.c
	multiview.c
)
include_directories(${OPENGL_INCLUDE_DIR})
include_directories(${GLUT_INCLUDE_DIR})
add_executable(city  ${SOURCES})
target_link_libraries(city ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})
target_link_libraries(city ${EXTRA_LIBS})
if(X11_PLATFORM)
	target_link_libraries(city ${X11_LIBRARIES})
endif()
install(TARGETS city
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)
