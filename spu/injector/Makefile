# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ../..

SPU = 1
include ${TOP}/arch.mk

SHARED = 1
LIBRARY = injectorspu
FILES = injectorspu \
	injectorspu_config \
	injectorspu_init

LIBRARIES = spuload crutil crmothership 

ifdef DARWIN
LIBRARIES += crpacker
endif

UNPACKS = 1

PRECOMP =
SLOP = $(PRECOMP)

LIB_DEFS += injectorspu.def
include ${TOP}/cr.mk
