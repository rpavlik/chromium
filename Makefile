# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = .

include $(TOP)/arch.mk

SUBDIRS = glapi_parser util mothership spu_loader packer state_tracker \
	unpacker dlm spu app_faker opengl_stub crserverlib crserver \
	crutapi crutclientapi crutproxy crutserver progs

include ${TOP}/cr.mk

