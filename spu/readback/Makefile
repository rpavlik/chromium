# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ../..

include ${TOP}/arch.mk

SHARED = 1
LIBRARY = readbackspu
FILES = readbackspu \
	readbackspu_config \
	readbackspu_init

ifdef WINDOWS
LIBRARIES = spuload crutil crmothership
else
LIBRARIES = spuload crutil crmothership
endif


ifdef BINUTIL_LINK_HACK
TRACKS_STATE = 0
endif

SLOP = $(PRECOMP)

LIB_DEFS += readbackspu.def

include ${TOP}/cr.mk
