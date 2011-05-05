set(SOURCES
	crutapi.c
)
add_library(crutapi SHARED ${SOURCES})
target_link_libraries(crutapi crmothership crutil)
