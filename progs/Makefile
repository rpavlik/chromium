# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ..

ifneq ($(ARCH), WIN_NT)
ifneq ($(ARCH), WIN_98)
ARCH=$(shell uname | sed -e 's/-//g')
endif
endif

ifeq ($(ARCH), CYGWIN_NT5.0)
ARCH=WIN_NT
endif

SUBDIRS = atlantis cubes fonttest kirchner extensions psubmit tiletest

ifneq ($(ARCH), WIN_NT)
SUBDIRS += glx
else
SUBDIRS += wgl
endif

include ${TOP}/cr.mk
