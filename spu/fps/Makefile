# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ../..

SPU = 1
SHARED = 1
LIBRARY = fpsspu
FILES = fpsspu \
	fpsspu_config \
	fpsspu_init

LIBRARIES = spuload crutil crmothership

PRECOMP =
SLOP = $(PRECOMP)

LIB_DEFS += fpsspu.def
include ${TOP}/cr.mk
