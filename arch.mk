# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

include $(TOP)/options.mk

ifneq ($(ARCH), WIN_NT)
ifneq ($(ARCH), WIN_98)
ARCH=$(shell uname | sed -e 's/-//g')
MACHTYPE=$(shell uname -m)
endif
endif

ifeq ($(ARCH), CYGWIN_NT5.0)
ARCH=WIN_NT
endif

ifeq ($(ARCH), CYGWIN_NT5.1)
ARCH=WIN_NT
endif

ifeq ($(MACHTYPE),i686)
MACHTYPE=i386
endif

ifeq ($(MACHTYPE),i586)
MACHTYPE=i386
endif

ifeq ($(MACHTYPE),i486)
MACHTYPE=i386
endif

ECHO := echo

include $(TOP)/config/$(ARCH).mk
