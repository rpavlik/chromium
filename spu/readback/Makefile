# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ../..
SPU = 1

include ${TOP}/arch.mk

SHARED = 1
LIBRARY = readbackspu
FILES = readbackspu \
	readbackspu_config \
	readbackspu_init

LIBRARIES = spuload crutil crmothership

ifdef BINUTIL_LINK_HACK
TRACKS_STATE = 0
endif

SLOP = $(PRECOMP)

LIB_DEFS += readbackspu.def

include ${TOP}/cr.mk
