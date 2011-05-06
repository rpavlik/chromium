include_directories(.)
set(SOURCES
	psubmit.c
)
add_executable(psubmit  ${SOURCES})
target_link_libraries(psubmit crutil crfaker crmothership spuload)
target_link_libraries(psubmit ${EXTRA_LIBS})
install(TARGETS psubmit
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)
