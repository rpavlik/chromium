include_directories(.)
set(SOURCES
	ball.c
	plyfile.c
	pmodel.c
	readply.c
)
add_executable(pmodel  ${SOURCES})
target_link_libraries(pmodel crutil crfaker)
target_link_libraries(pmodel ${EXTRA_LIBS})
install(TARGETS pmodel
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)
