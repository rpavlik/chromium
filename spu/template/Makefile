# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ../..

SHARED = 1
LIBRARY = templatespu
FILES = templatespu \
	templatespu_config \
	templatespu_init

LIBRARIES = spuload crutil crmothership

SLOP = $(PRECOMP)

LIB_DEFS += templatespu.def
include ${TOP}/cr.mk
