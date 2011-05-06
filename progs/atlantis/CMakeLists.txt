include_directories(.)
set(SOURCES
	atlantis.c
	dolphin.c
	shark.c
	swim.c
	whale.c
)
add_executable(atlantis  ${SOURCES})
target_link_libraries(atlantis ${EXTRA_LIBS})
install(TARGETS atlantis
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)
