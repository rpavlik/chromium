# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ..

include ${TOP}/arch.mk

SUBDIRS = atlantis cubes fonttest kirchner extensions psubmit retile \
	statecopytest tiletest

ifeq ($(QT), 1)
SUBDIRS += cr-recorder
endif

ifeq ($(ARCH), WIN_NT)
SUBDIRS += wgl
else
SUBDIRS += glx threadtest city
endif

include ${TOP}/cr.mk
