# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ../..

include ${TOP}/arch.mk

SPU = 1
SHARED = 1
LIBRARY = trackerspu
FILES = trackerspu \
	trackerspu_config \
	trackerspu_init \
	trackerspu_udp

LIBRARIES = spuload crutil crmothership

PRECOMP =
SLOP = $(PRECOMP)

LIB_DEFS += trackerspu.def

ifeq ($(ARCH), WIN_NT)
LDFLAGS += user32.lib ws2_32.lib
endif

include ${TOP}/cr.mk
