include_directories(.)
set(SOURCES
	disttexture.c
)
add_executable(disttexture  ${SOURCES})
target_link_libraries(disttexture ${EXTRA_LIBS})
install(TARGETS disttexture
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)
