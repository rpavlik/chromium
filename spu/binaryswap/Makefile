# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ../..

OPENGL = 1
SHARED = 1
LIBRARY = binaryswapspu
FILES = binaryswapspu \
	binaryswapspu_config \
	binaryswapspu_init

LIBRARIES = spuload crutil crmothership

PRECOMP =
SLOP = $(PRECOMP)

LIB_DEFS += binaryswapspu.def
include ${TOP}/cr.mk
