include_directories(.)
set(GENERATED
	feedbackspu.c
	feedbackspu_state.c
	feedbackspu_proto.h
)
# TODO: generate these files!


set(SOURCES
	feedbackspu_config.c
	feedbackspu_init.c
	${CMAKE_CURRENT_BINARY_DIR}/feedbackspu.c
	${CMAKE_CURRENT_BINARY_DIR}/feedbackspu_state.c
	${CMAKE_CURRENT_BINARY_DIR}/feedbackspu_proto.h
	feedback.def
)
add_library(feedbackspu SHARED ${SOURCES})
target_link_libraries(feedbackspu spuload crutil crmothership crstate)
target_link_libraries(feedbackspu ${EXTRA_LIBS})
install(TARGETS feedbackspu
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)
