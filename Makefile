# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = .

include $(TOP)/arch.mk

SUBDIRS = glapi_parser util mothership spu_loader packer state_tracker \
	unpacker spu app_faker opengl_stub crserverlib crserver \
	crutapi crutclientapi crutproxy progs

ifneq ($(ARCH), WIN_NT)
SUBDIRS += crutserver
endif

include ${TOP}/cr.mk

