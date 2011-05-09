include_directories(.)
set(SOURCES
	crutclientapi.c
	crutclientapi.def
)
add_library(crutclientapi SHARED ${SOURCES})
target_link_libraries(crutclientapi crutapi crfaker crmothership crutil)
target_link_libraries(crutclientapi ${EXTRA_LIBS})
install(TARGETS crutclientapi
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)
