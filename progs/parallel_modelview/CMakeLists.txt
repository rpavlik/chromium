set(SOURCES
	ball.c
	plyfile.c
	pmodel.c
	readply.c
)
add_executable(pmodel ${SOURCES})
target_link_libraries(pmodel crutil crfaker)
