# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = .
SUBDIRS = glapi_parser util mothership packer spu_loader state_tracker \
	unpacker spu app_faker opengl_stub crserver app_stub progs

include ${TOP}/cr.mk
