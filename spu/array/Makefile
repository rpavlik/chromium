# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ../..

SHARED = 1
LIBRARY = arrayspu
FILES = arrayspu \
	arrayspu_config \
	arrayspu_init

TRACKS_STATE = 1

LIBRARIES = spuload crutil crmothership

SLOP = $(PRECOMP)

LIB_DEFS += arrayspu.def
include ${TOP}/cr.mk
