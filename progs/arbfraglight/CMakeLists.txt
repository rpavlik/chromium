include_directories(.)
set(SOURCES
	arbfraglight.c
)
include_directories(${OPENGL_INCLUDE_DIR})
include_directories(${GLUT_INCLUDE_DIR})
add_executable(arbfraglight  ${SOURCES})
target_link_libraries(arbfraglight crutil crfaker crmothership spuload ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})
target_link_libraries(arbfraglight ${EXTRA_LIBS})
if(X11_PLATFORM)
	target_link_libraries(arbfraglight ${X11_LIBRARIES})
endif()
install(TARGETS arbfraglight
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)
