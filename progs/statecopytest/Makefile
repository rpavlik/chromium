# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ../..
PROGRAM = statecopytest
FILES = statecopytest

ifdef WINDOWS
TRACKS_STATE = 1
LIBRARIES = crutil spuload
else
LIBRARIES = crutil spuload crstate
endif


include ${TOP}/cr.mk
