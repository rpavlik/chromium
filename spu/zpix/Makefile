# Copyright (c) 2003, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ../..

SPU = 1
SHARED = 1
LIBRARY = zpixspu
FILES =	zpixspu_config \
	zpixspu_init \
	zpixspu

LIBRARIES = spuload crutil crmothership
LDFLAGS =  -lz

PRECOMP =
SLOP = $(PRECOMP)

LIB_DEFS += zpixspu.def
include ${TOP}/cr.mk
