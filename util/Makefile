TOP = ..
LIBRARY = crutil

FILES = dll error mem string

include ${TOP}/cr.mk

ifdef WINDOWS
LIB_DEFS = util.def
LDFLAGS += user32.lib ws2_32.lib
endif
