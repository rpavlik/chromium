TOP = ..
ifneq ($(ARCH), WIN_NT)
ifneq ($(ARCH), WIN_98)
ARCH=$(shell uname | sed -e 's/-//g')
endif
endif

ifeq ($(ARCH),CYGWIN_NT5.0)
ARCH=WIN_NT
endif

SHARED = 1
LIBRARY = crutil
FILES = bufpool devnull dll environment error mem net pixel string tcpip url

ifeq ($(ARCH),Linux)
CFLAGS += -DGM_SUPPORT
FILES += gm
LDFLAGS += -lgm
endif

LIB_DEFS = util.def

ifeq ($(ARCH), WIN_NT)
CFLAGS += -DGM_SUPPORT
FILES += gm
LDFLAGS += user32.lib ws2_32.lib gm.lib
else
LDFLAGS += -ldl
endif

include ${TOP}/cr.mk
