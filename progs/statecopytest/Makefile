# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ../..

include ${TOP}/arch.mk

PROGRAM = statecopytest
FILES = statecopytest

ifdef WINDOWS
TRACKS_STATE = 1
LIBRARIES = crutil spuload crmothership
else
LIBRARIES = crutil spuload crmothership crstate
endif

ifdef BINUTIL_LINK_HACK
TRACKS_STATE = 1
LIBRARIES -= crstate
endif

include ${TOP}/cr.mk
