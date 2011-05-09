include_directories(.)
if(WIN32)
	set(MOTHERSHIP_DIR mothership/)
else()
	set(MOTHERSHIP_DIR share/cr/mothership/)
endif()

add_subdirectory(client)
add_subdirectory(reset)
add_subdirectory(quit)
add_subdirectory(server)
add_subdirectory(status)

install(DIRECTORY
	configs
	server
	tools
	prshd
	DESTINATION ${MOTHERSHIP_DIR}
	COMPONENT mothership
	FILES_MATCHING
	PATTERN *.py
	PATTERN *.conf
	PATTERN *.crsite)
