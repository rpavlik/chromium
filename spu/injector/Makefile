# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ../..

SHARED = 1
LIBRARY = injectorspu
FILES = injectorspu \
	injectorspu_config \
	injectorspu_init

LIBRARIES = spuload crutil crmothership

UNPACKS = 1

PRECOMP =
SLOP = $(PRECOMP)

LIB_DEFS += injectorspu.def
include ${TOP}/cr.mk
