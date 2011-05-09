include_directories(.)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(SOURCES
	feedbackspu_config.c
	feedbackspu_init.c
	${CMAKE_CURRENT_BINARY_DIR}/feedbackspu.c
	${CMAKE_CURRENT_BINARY_DIR}/feedbackspu_state.c
	${CMAKE_CURRENT_BINARY_DIR}/feedbackspu_proto.h
	feedback.def
)

generate_with_python(feedback.py
	feedbackspu.c
	feedback_special
	feedback_state_special
	select_special
	${CMAKE_CURRENT_BINARY_DIR}/feedbackspu_proto.h
	${APIFILES})
generate_with_python(feedback_state.py
	feedbackspu_state.c
	feedback_state_special
	${APIFILES})
generate_with_python(feedbackspu_proto.py
	feedbackspu_proto.h
	feedback_special
	feedback_state_special
	select_special
	${APIFILES})

add_library(feedbackspu SHARED ${SOURCES})
target_link_libraries(feedbackspu spuload crutil crmothership crstate)
target_link_libraries(feedbackspu ${EXTRA_LIBS})
install(TARGETS feedbackspu
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)
