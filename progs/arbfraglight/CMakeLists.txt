include_directories(.)
set(SOURCES
	arbfraglight.c
)
add_executable(arbfraglight  ${SOURCES})
target_link_libraries(arbfraglight crutil crfaker crmothership spuload)
target_link_libraries(arbfraglight ${EXTRA_LIBS})
install(TARGETS arbfraglight
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)
