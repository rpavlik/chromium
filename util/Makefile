TOP = ..
LIBRARY = crutil
FILES = bufpool dll error mem net string tcpip url

LIB_DEFS = util.def

include ${TOP}/cr.mk

ifdef WINDOWS
LDFLAGS += user32.lib ws2_32.lib
endif
