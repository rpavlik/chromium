# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ../..

include $(TOP)/arch.mk

PROGRAM = dmx_config
FILES = getconfig

ifdef DMX_LIBDIR
LDFLAGS += -L${DMX_LIBDIR}
endif
LDFLAGS += -ldmx -lXext -lX11
ifdef DMX_INCDIR
CFLAGS += -I${DMX_INCDIR}
endif

LIBRARIES = 

include $(TOP)/cr.mk
