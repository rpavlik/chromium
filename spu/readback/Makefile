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
TRACKS_STATE = 1
LIBRARIES = spuload crutil crmothership
else
# use static libcrstate.a on everything else
LIBRARIES = spuload crutil crmothership crstate
endif


SLOP = $(PRECOMP)

LIB_DEFS += readbackspu.def

include ${TOP}/cr.mk
