# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ../..

SPU = 1
include ${TOP}/arch.mk

SHARED = 1
LIBRARY = saveframespu
FILES = saveframespu \
	saveframespu_config \
	saveframespu_init

LIBRARIES = spuload crutil crmothership

SLOP = $(PRECOMP)

LIB_DEFS += saveframespu.def

ifeq ($(ARCH),Linux)
CFLAGS += -DJPEG
LIBRARIES += jpeg
endif

include ${TOP}/cr.mk
