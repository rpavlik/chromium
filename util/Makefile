TOP = ..
SHARED = 1
LIBRARY = crutil
FILES = bufpool devnull dll error mem net pixel string tcpip url

LIB_DEFS = util.def

include ${TOP}/cr.mk

ifdef WINDOWS
LDFLAGS += user32.lib ws2_32.lib
else
LDFLAGS += -ldl
endif
