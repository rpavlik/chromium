# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ../..
SPU = 1

include ${TOP}/arch.mk

SHARED = 1
LIBRARY = grabberspu
FILES = grabberspu \
	grabberspu_config \
	grabberspu_init

LIBRARIES += crutil crmothership spuload

LIB_DEFS += grabber.def
include ${TOP}/cr.mk
