# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ..

include ${TOP}/arch.mk

SHARED = 1
LIBRARY = crutil
FILES = bbox \
	bufpool \
	devnull \
	dll \
	endian \
	environment \
	error \
	filenet \
	hash \
	mem \
	net \
	pixel \
	rand \
	string \
	threads \
	tcpip \
	timer \
	url
PRECOMP = debug_opcodes.c
SLOP += ${PRECOMP}

#ifeq ($(ARCH),Linux)
#CFLAGS += -DGM_SUPPORT
#FILES += gm
#LDFLAGS += -lgm
#endif

ifeq ($(ARCH), SunOS)
LDFLAGS+= -lsocket -lnsl
endif

LIB_DEFS = util.def

ifeq ($(ARCH), WIN_NT)
ifdef (WANT_GM)
CFLAGS += -DGM_SUPPORT
FILES += gm
LDFLAGS += gm.lib
endif
LDFLAGS += user32.lib ws2_32.lib
else
ifeq ($(ARCH),FreeBSD)
LDFLAGS += -lpthread
else
ifndef IRIX_64BIT
LDFLAGS += -ldl -lpthread
else
LDFLAGS += -lpthread
endif
endif
endif

include ${TOP}/cr.mk

debug_opcodes.c: debug_opcodes.py ../packer/opcode_special
	@$(ECHO) Making the opcode debugger...
	@$(PYTHON) debug_opcodes.py > $@
