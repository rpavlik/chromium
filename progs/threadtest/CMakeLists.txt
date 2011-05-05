set(SOURCES
	threadtest.c
)
add_executable(threadtest ${SOURCES})
target_link_libraries(threadtest crutil crfaker crmothership spuload)
