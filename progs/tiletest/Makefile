# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

ifneq ($(ARCH), WIN_NT)
ifneq ($(ARCH), WIN_98)
ARCH=$(shell uname | sed -e 's/-//g')
MACHTYPE=$(shell uname -m)
endif
endif

TOP = ../..

PROGRAM = tiletest
OPENGL = 1
GLUT = 1

FILES = tiletest

ifeq ($(ARCH), SunOS)
LDFLAGS += -lcrfaker 
endif

include ${TOP}/cr.mk
