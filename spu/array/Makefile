# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ../..

SPU = 1
include ${TOP}/arch.mk

SHARED = 1
LIBRARY = arrayspu
FILES = arrayspu \
	arrayspu_config \
	arrayspu_init

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

SLOP = $(PRECOMP)

LIB_DEFS += arrayspu.def

include ${TOP}/cr.mk
