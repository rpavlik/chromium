# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ../..

SPU = 1
SHARED = 1
LIBRARY = dist_texturespu
FILES = dist_texturespu \
	dist_texturespu_config \
	dist_texturespu_texture \
	dist_texturespu_init

LIBRARIES = spuload crutil crmothership

PRECOMP =
SLOP = $(PRECOMP)

LIB_DEFS += dist_texturespu.def

include ${TOP}/cr.mk
