include_directories(.)
add_subdirectory(client)
add_subdirectory(reset)
add_subdirectory(quit)
add_subdirectory(server)
add_subdirectory(status)

install(DIRECTORY
	configs
	#client
	server
	tools
	prshd
	DESTINATION share/cr/mothership/
	COMPONENT mothership)
