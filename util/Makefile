TOP = ..
ifneq ($(ARCH), WIN_NT)
ifneq ($(ARCH), WIN_98)
ARCH=$(shell uname | sed -e 's/-//g')
endif
endif

SHARED = 1
LIBRARY = crutil
FILES = bufpool devnull dll error mem net pixel string tcpip url

ifeq ($(ARCH),Linux)
CFLAGS += -DGM_SUPPORT
FILES += gm
LDFLAGS += -lgm
endif

LIB_DEFS = util.def

include ${TOP}/cr.mk

ifdef WINDOWS
CFLAGS += -DGM_SUPPORT
FILES += gm
LDFLAGS += user32.lib ws2_32.lib gm.lib
endif

ifndef WINDOWS
LDFLAGS += -ldl
endif
