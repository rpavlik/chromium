# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ../..

PROGRAM = pmodel
FILES = ball plyfile pmodel readply


LIBRARIES = crutil crfaker

include ${TOP}/cr.mk

ifeq ($(ARCH),WIN_NT)
LDFLAGS += GLU32.lib
else
LDFLAGS += -lGLU
endif
