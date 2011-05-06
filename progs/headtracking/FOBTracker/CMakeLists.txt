include_directories(.)
set(SOURCES
	moc_fobtracker.c
	FlockOfBird.c
	fobtracker.c
	main.c
	RotationPreview.c
	udp.c
)
add_executable(FOBTracker  ${SOURCES})
target_link_libraries(FOBTracker ${EXTRA_LIBS})
install(TARGETS FOBTracker
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)
