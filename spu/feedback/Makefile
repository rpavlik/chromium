# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ../..

SPU=1
include ${TOP}/arch.mk


SHARED = 1
LIBRARY = feedbackspu
FILES = feedbackspu \
	feedbackspu_config \
	feedbackspu_state \
	feedbackspu_init

ifdef WINDOWS
TRACKS_STATE = 1
LIBRARIES = spuload crutil crmothership
else
LIBRARIES = spuload crutil crmothership crstate
endif

ifdef BINUTIL_LINK_HACK
TRACKS_STATE = 1
LIBRARIES -= crstate
endif


PRECOMP = feedbackspu.c feedbackspu_state.c feedbackspu_proto.h
SLOP = $(PRECOMP)

LIB_DEFS += feedback.def

include ${TOP}/cr.mk

APIFILES = ../../glapi_parser/apiutil.py ../../glapi_parser/APIspec.txt

feedbackspu.c: feedback.py feedback_special select_special feedback_state_special feedbackspu_proto.h $(APIFILES)
	@$(ECHO) Building the Feedback \& Selection SPU
	@$(PYTHON) feedback.py > $@

feedbackspu_state.c: feedback_state.py feedback_state_special $(APIFILES)
	@$(ECHO) Building the Feedback \& Selection State Tracker
	@$(PYTHON) feedback_state.py > $@

feedbackspu_proto.h: feedbackspu_proto.py feedback_special select_special feedback_state_special $(APIFILES)
	@$(ECHO) Building the Feedback \& Selection SPU prototypes header
	@$(PYTHON) feedbackspu_proto.py > $@

