
TOP = ..

include ${TOP}/arch.mk

SHARED = 1
LIBRARY = dmxutil
FILES = dmx

include ${TOP}/cr.mk

LDFLAGS += -lXmu -lX11
ifeq ($(USE_DMX), 1)
    ifdef DMX_LIB
        LDFLAGS += ${DMX_LIB}
    endif
    LDFLAGS += -ldmx
    ifdef DMX_INCDIR
         CFLAGS += -I${DMX_INCDIR}
    endif
endif


