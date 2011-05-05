set(SOURCES
	crutclientapi.c
)
add_library(crutclientapi SHARED ${SOURCES})
target_link_libraries(crutclientapi crutapi crfaker crmothership crutil)
