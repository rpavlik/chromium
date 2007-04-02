# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

UIC = $(QTDIR)/bin/uic
MOC = $(QTDIR)/bin/moc

TOP = ../../..

PROGRAM = FOBTracker
FILES = moc_fobtracker FlockOfBird fobtracker main RotationPreview udp

CXXFLAGS += -I$(QTDIR)/include
CXXFLAGS += -I$(QTDIR)/include/QtCore
CXXFLAGS += -I$(QTDIR)/include/QtGui
CXXFLAGS += -I$(QTDIR)/include/QtNetwork
LDFLAGS += /LIBPATH:$(QTDIR)/lib

ifeq ($(RELEASE), 1)
  LDFLAGS += QtCore4.lib QtGui4.lib QtNetwork4.lib bird.lib  
else
  LDFLAGS += QtCored4.lib QtGuid4.lib QtNetworkd4.lib bird.lib  
endif


include ${TOP}/cr.mk

ui_fobtracker.h: fobtracker.ui
	$(UIC) -o ui_fobtracker.h fobtracker.ui 

moc_fobtracker.cpp: fobtracker.ui ui_fobtracker.h
	$(MOC) fobtracker.h > moc_fobtracker.cpp
