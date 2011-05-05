set(SOURCES
	feedbackspu.c
	feedbackspu_config.c
	feedbackspu_state.c
	feedbackspu_init.c
)
set(GENERATED
	feedbackspu.c
	feedbackspu_state.c
	feedbackspu_proto.h
)
# TODO: generate these files!


add_library(feedbackspu SHARED ${SOURCES})
target_link_libraries(feedbackspu spuload crutil crmothership crstate)
