# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = .
SUBDIRS = glapi_parser util mothership spu_loader packer state_tracker \
	unpacker spu app_faker opengl_stub crserverlib crserver \
	crutapi crutclientapi crutserver crutproxy progs

include ${TOP}/cr.mk

