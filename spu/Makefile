# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ..

include ${TOP}/arch.mk

SUBDIRS = tester options
SUBDIRS += error nop pack passthrough print readback render template tilesort
SUBDIRS += saveframe comm array binaryswap hiddenline fps wet perf dist_texture
SUBDIRS += feedback simplequery motionblur injector expando matte grabber
#SUBDIRS += broadcast
SUBDIRS += grey16
SUBDIRS += tgrb

ifeq ($(USE_VNC), 1)
ifeq ($(THREADSAFE), 1)
SUBDIRS += replicate
endif
endif

ifeq ($(ARCH), WIN_NT)
# zpix totally broken on NT
else
SUBDIRS += zpix
endif

include ${TOP}/cr.mk
