TOP = ..
SHARED = 1
LIBRARY = crutil
FILES = bufpool devnull dll error gm mem net pixel string tcpip url

LIB_DEFS = util.def

include ${TOP}/cr.mk

ifdef WINDOWS
CFLAGS += -DGM_SUPPORT
FILES += gm
LDFLAGS += user32.lib ws2_32.lib gm.lib
endif

ifdef LINUX
CFLAGS += -DGM_SUPPORT
FILES += gm
LDFLAGS += -lgm
endif

ifndef WINDOWS
LDFLAGS += -ldl
endif
