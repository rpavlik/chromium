# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ../..

include ${TOP}/arch.mk

PROGRAM = tiletest
OPENGL = 1
GLUT = 1

FILES = tiletest

ifeq ($(ARCH), SunOS)
LDFLAGS += -lcrfaker 
endif

ifeq ($(ARCH), Darwin)
LIBRARIES = crutil crfaker crmothership spuload
FORCE_FAKER = 1
endif

include ${TOP}/cr.mk
