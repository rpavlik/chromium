# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ../..

PROGRAM = cr-recorder
FILES = cr-recorder recorderdialog recorderdialogimpl moc_recorderdialog moc_recorderdialogimpl moc_cr-recorder

CXXFLAGS += -I$(QTDIR)/include
LDFLAGS += -L$(QTDIR)/lib -lqt

LIBRARIES = crutil crpacker

include ${TOP}/cr.mk

recorderdialog.h: cr-recorder-dialog.ui
	$(UIC) cr-recorder-dialog.ui > recorderdialog.h

recorderdialog.cxx: cr-recorder-dialog.ui recorderdialog.h
	$(UIC) -impl recorderdialog.h cr-recorder-dialog.ui > recorderdialog.cxx

