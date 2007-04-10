# Copyright (c) 2007, Tungsten Graphics, Inc.
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ../..

SPU = 1
SHARED = 1
LIBRARY = windowtrackerspu
FILES = windowtrackerspu \
	windowtrackerspu_config \
	windowtrackerspu_init

LIBRARIES = spuload crutil crmothership

PRECOMP =
SLOP = $(PRECOMP)

LIB_DEFS += windowtrackerspu.def
include ${TOP}/cr.mk
