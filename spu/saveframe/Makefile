# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ../..

SHARED = 1
LIBRARY = saveframespu
FILES = saveframespu \
	saveframespu_config \
	saveframespu_init

LIBRARIES = spuload crutil crmothership

SLOP = $(PRECOMP)

LIB_DEFS += saveframespu.def
include ${TOP}/cr.mk
