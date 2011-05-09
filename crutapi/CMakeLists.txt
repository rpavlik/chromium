include_directories(.)
set(SOURCES
	crutapi.c
	crutapi.def
)
add_library(crutapi SHARED ${SOURCES})
target_link_libraries(crutapi crmothership crutil)
target_link_libraries(crutapi ${EXTRA_LIBS})
install(TARGETS crutapi
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)
