include_directories(.)

set(SOURCES
	bbox.c
	bufpool.c
	calllists.c
	devnull.c
	dll.c
	endian.c
	environment.c
	error.c
	filenet.c
	hash.c
	hull.c
	list.c
	matrix.c
	mem.c
	net.c
	pixel.c
	process.c
	rand.c
	string.c
	threads.c
	tcpip.c
	udptcpip.c
	timer.c
	url.c
	warp.c
	${CMAKE_CURRENT_BINARY_DIR}/debug_opcodes.c
)
generate_with_python(debug_opcodes.py debug_opcodes.c ${APIFILES})

add_library(crutil SHARED ${SOURCES})
target_link_libraries(crutil ${EXTRA_LIBS})
install(TARGETS crutil
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)
add_dependencies(crutil common_includes)