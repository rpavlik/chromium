# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ..

SUBDIRS = tester options
SUBDIRS += error nop pack passthrough print readback render template tilesort
SUBDIRS += saveframe comm array binaryswap perf hiddenline fps wet

ifneq ($(ARCH), WIN_NT)
SUBDIRS += dist_texture
endif

include ${TOP}/cr.mk
