# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ../..

include ${TOP}/arch.mk

SHARED = 1
LIBRARY = renderspu
FILES = renderspu \
	renderspu_config \
	renderspu_init

ifeq ($(ARCH), WIN_NT)
FILES += renderspu_wgl
else
FILES += renderspu_glx
endif

# Ugh, we only need the state tracker for renderspuGetString(), oh well.
ifdef WINDOWS
TRACKS_STATE = 1
LIBRARIES += crutil crmothership spuload
else
LIBRARIES += crutil crmothership spuload crstate
endif


SLOP = $(PRECOMP)

LIB_DEFS += render.def
include ${TOP}/cr.mk

ifdef WINDOWS
LDFLAGS += user32.lib gdi32.lib
else
LDFLAGS += -lXmu -lX11
endif
