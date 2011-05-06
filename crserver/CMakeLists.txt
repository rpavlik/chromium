include_directories(.)
set(SOURCES
	main.c
)
add_executable(crserver  ${SOURCES})
target_link_libraries(crserver crserverlib spuload crmothership crstate crpacker crutil crserver_crunpacker_copy)
target_link_libraries(crserver ${EXTRA_LIBS})
install(TARGETS crserver
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)
