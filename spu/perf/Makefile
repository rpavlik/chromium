# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ../..

SHARED = 1
LIBRARY = perfspu
FILES = perfspu \
	perfspu_config \
	perfspu_misc \
	perfspu_init

LIBRARIES = spuload crutil crmothership

PRECOMP =
SLOP = $(PRECOMP)

LIB_DEFS += perfspu.def
include ${TOP}/cr.mk
