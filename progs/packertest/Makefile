# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ../../
include ${TOP}/arch.mk

FILES = \
	packertest0 \
	packertest100 \
	packertest125 \
	packertest150 \
	packertest175 \
	packertest200 \
	packertest225 \
	packertest250 \
	packertest25 \
	packertest275 \
	packertest300 \
	packertest325 \
	packertest350 \
	packertest375 \
	packertest400 \
	packertest425 \
	packertest450 \
	packertest475 \
	packertest500 \
	packertest50 \
	packertest525 \
	packertest550 \
	packertest575 \
	packertest75 \
	packertestBlendFuncSeparateEXT \
	packertest \
	packertestAttrib \
	packertestColorTable \
	packertestColorTableEXT \
	packertestCombinerInputNV \
	packertestCompressedTexARB \
	packertestTexFuncs \
	packertestTexImage1D \
	packertestTexImage2D \
	packertestTexImage3D \
	packertestTexSubImage1D \
	packertestTexSubImage2D \
	packertestTexSubImage3D \
	packertestBeginEnd \
	packertestBufferARB \
	packertestCRfuncs \
	packertestFenceNV \
	packertestLists \
	packertestPointParamARB \
	packertestProgramARB \
	packertestProgramNV \
	packertestQueriesARB

PROGRAM = packertest
GLUT = 1
OPENGL = 1
LIBRARIES = crutil crfaker crmothership spuload
LDFLAGS += -lcrutil -lcrfaker -lcrmothership -lspuload


PRECOMP = \
	packertest0.c \
	packertest100.c \
	packertest125.c \
	packertest150.c \
	packertest175.c \
	packertest200.c \
	packertest225.c \
	packertest250.c \
	packertest25.c \
	packertest275.c \
	packertest300.c \
	packertest325.c \
	packertest350.c \
	packertest375.c \
	packertest400.c \
	packertest425.c \
	packertest450.c \
	packertest475.c \
	packertest500.c \
	packertest50.c \
	packertest525.c \
	packertest550.c \
	packertest575.c \
	packertest75.c \
	packertestBlendFuncSeparateEXT.c \
	packertest.c \
	packertestColorTable.c \
	packertestColorTableEXT.c \
	packertestCombinerInputNV.c \
	packertest.h \
	packertestTexImage1D.c \
	packertestTexImage2D.c \
	packertestTexImage3D.c \
	packertestTexSubImage1D.c \
	packertestTexSubImage2D.c \
	packertestTexSubImage3D.c

SLOP = $(PRECOMP)
include $(TOP)/cr.mk

APIFILES = ../../glapi_parser/apiutil.py ../../glapi_parser/APIspec.txt

#$(PRECOMP): packertest.py   packertest_special
$(PRECOMP): packertest.py  $(APIFILES)  packertest_special
	@$(ECHO) Making the PackerTest functions
	@$(PYTHON) packertest.py -v -d

