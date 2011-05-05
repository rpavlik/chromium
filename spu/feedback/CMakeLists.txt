set(SOURCES
	feedbackspu.c
	feedbackspu_config.c
	feedbackspu_state.c
	feedbackspu_init.c
)
add_library(feedbackspu SHARED ${SOURCES})
target_link_libraries(feedbackspu spuload crutil crmothership crstate)
