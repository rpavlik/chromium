# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ../..

SHARED = 1
LIBRARY = readbackspu
FILES = readbackspu \
	readbackspu_config \
	readbackspu_init
TRACKS_STATE = 1

LIBRARIES = spuload crutil crmothership

SLOP = $(PRECOMP)

LIB_DEFS += readbackspu.def
include ${TOP}/cr.mk
