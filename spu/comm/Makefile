# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ../..

SHARED = 1
LIBRARY = commspu
FILES = commspu \
	commspu_config \
	commspu_init

LIBRARIES = spuload crutil crmothership

SLOP = $(PRECOMP)

LIB_DEFS += commspu.def
include ${TOP}/cr.mk
