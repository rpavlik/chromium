# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ../..

SHARED = 1
LIBRARY = binaryswapspu
FILES = binaryswapspu \
	binaryswapspu_config \
	binaryswapspu_init
TRACKS_STATE = 1

LIBRARIES = spuload crutil crmothership

SLOP = $(PRECOMP)

LIB_DEFS += binaryswapspu.def
include ${TOP}/cr.mk
