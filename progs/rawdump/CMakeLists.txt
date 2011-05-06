include_directories(.)
set(SOURCES
	rawdump.c
)
add_executable(rawdump  ${SOURCES})
target_link_libraries(rawdump crutil)
target_link_libraries(rawdump ${EXTRA_LIBS})
install(TARGETS rawdump
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)
