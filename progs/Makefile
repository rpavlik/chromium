# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ..

include ${TOP}/arch.mk

SUBDIRS = \
	arbfraglight \
	atlantis \
	bboxtest \
	city \
	cubes \
	disttexture \
	dlisttest \
	extensions \
	fonttest \
	fraglight \
	kirchner \
	netperf \
	nullapp \
	packperf \
	psubmit \
	psubmit_crut \
	retile \
	spheres \
	statecopytest \
	stereocube \
	tiletest \
	viewports


ifeq ($(THREADSAFE), 1)
SUBDIRS += threadtest
endif

ifeq ($(QT), 1)
SUBDIRS += cr-recorder
endif

ifeq ($(ARCH), WIN_NT)
SUBDIRS += wgl
else
ifndef DARWIN
SUBDIRS += glx dlistperf1 dlistperf2 packertest
endif
endif

include ${TOP}/cr.mk
