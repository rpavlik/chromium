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

ifeq ($(ARCH), CYGWIN_NT5.1)
ARCH=WIN_NT
endif

SUBDIRS = atlantis cubes fonttest kirchner extensions psubmit tiletest

ifeq ($(ARCH), WIN_NT)
SUBDIRS += wgl
else
SUBDIRS += glx threadtest
endif

include ${TOP}/cr.mk
