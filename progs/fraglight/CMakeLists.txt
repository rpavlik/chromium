include_directories(.)
set(SOURCES
	fraglight.c
)
include_directories(${OPENGL_INCLUDE_DIR})
include_directories(${GLUT_INCLUDE_DIR})
add_executable(fraglight  ${SOURCES})
target_link_libraries(fraglight crutil crfaker crmothership spuload ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})
target_link_libraries(fraglight ${EXTRA_LIBS})
if(X11_PLATFORM)
	target_link_libraries(fraglight ${X11_LIBRARIES})
endif()
install(TARGETS fraglight
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)
