# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ..

include ${TOP}/arch.mk

SUBDIRS = arbfraglight atlantis cubes fonttest kirchner extensions \
	psubmit retile \
	statecopytest tiletest city psubmit_crut bboxtest viewports \
	fraglight stereocube packperf spheres

ifeq ($(THREADSAFE), 1)
SUBDIRS += threadtest
endif

ifeq ($(QT), 1)
SUBDIRS += cr-recorder
endif

ifeq ($(ARCH), WIN_NT)
SUBDIRS += wgl
else
SUBDIRS += glx
endif

include ${TOP}/cr.mk
