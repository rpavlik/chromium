# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ../..

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

PRECOMP = feedbackspu.c feedback.h
SLOP = $(PRECOMP)

LIB_DEFS += feedback.def

include ${TOP}/cr.mk

feedbackspu.c: feedback.py feedback_special select_special feedback_state_special
	@$(ECHO) Building the Feedback \& Selection SPU
	@$(PYTHON) feedback.py > $@

feedbackspu_state.c: feedback_state.py feedback_state_special
	@$(ECHO) Building the Feedback \& Selection State Tracker
	@$(PYTHON) feedback_state.py > $@

feedback.h: feedback_funcs.py feedback_special select_special
	@$(ECHO) Making feedback.h
	@$(PYTHON) feedback_funcs.py > $@
