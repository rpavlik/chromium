TOP = ..
SHARED = 1
LIBRARY = crutil
FILES = bufpool devnull dll error mem net pixel string tcpip url

LIB_DEFS = util.def

include ${TOP}/cr.mk

ifdef WINDOWS
CFLAGS += -DGM_SUPPORT
FILES += gm
LDFLAGS += user32.lib ws2_32.lib gm.lib
endif

ifeq ($(ARCH),Linux)
CFLAGS += -DGM_SUPPORT
FILES += gm
LDFLAGS += -lgm
endif

ifndef WINDOWS
LDFLAGS += -ldl
endif
