# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ..

include ${TOP}/arch.mk

# Uncomment for Quadrics Elan3 interface support
#TEAC_SUPPORT = 1
#TCSCOMM_SUPPORT = 1

# Uncomment for Myrinet Support
#GM_SUPPORT = 1

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
	hull \
	mem \
	net \
	pixel \
	process \
	rand \
	string \
	threads \
	tcpip \
	udptcpip \
	timer \
	url \
	warp
PRECOMP = debug_opcodes.c
SLOP += ${PRECOMP}

ifeq ($(GM_SUPPORT), 1)
ifeq ($(ARCH),Linux)
CFLAGS += -DGM_SUPPORT
FILES += gm
LDFLAGS += -lgm
endif
ifeq ($(ARCH), WIN_NT)
CFLAGS += -DGM_SUPPORT
FILES += gm
LDFLAGS += gm.lib
endif
endif

ifeq ($(ARCH), SunOS)
LDFLAGS+= -lsocket -lnsl
endif

LIB_DEFS = util.def

ifeq ($(ARCH), WIN_NT)
LDFLAGS += user32.lib ws2_32.lib
else
ifeq ($(ARCH),FreeBSD)
LDFLAGS += -lpthread
else
ifeq ($(ARCH), OSF1)
LDFLAGS += -lpthread
else
ifndef IRIX_64BIT
LDFLAGS += -ldl -lpthread
else
LDFLAGS += -lpthread
endif
endif
endif
endif

ifeq ($(TEAC_SUPPORT), 1)
CFLAGS += -DTEAC_SUPPORT -DCHROMIUM
ifeq ($(ARCH), OSF1)
CFLAGS += -I/usr/opt/rms/include
endif
LDFLAGS += -lelan3
FILES += libteac teac
endif

ifeq ($(TCSCOMM_SUPPORT), 1)
CFLAGS += -DTCSCOMM_SUPPORT -DCHROMIUM -I/usr/opt/rms/include
LDFLAGS += -lelan3
FILES += tcscomm 
endif

include ${TOP}/cr.mk

debug_opcodes.c: debug_opcodes.py ../packer/opcode_special
	@$(ECHO) Making the opcode debugger...
	@$(PYTHON) debug_opcodes.py > $@
