# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ../..

include ${TOP}/arch.mk

SHARED = 1
LIBRARY = binaryswapspu
FILES = binaryswapspu \
	binaryswapspu_config \
	binaryswapspu_init

ifdef WINDOWS
LIBRARIES = spuload crutil crmothership
else
LIBRARIES = spuload crutil crmothership crstate
endif

SLOP = $(PRECOMP)

LIB_DEFS += binaryswapspu.def

include ${TOP}/cr.mk
