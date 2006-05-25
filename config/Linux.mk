# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.


G++-INCLUDE-DIR = /usr/include/g++
CXX = g++
CC = gcc

# Mike Houston reports 20-30% speed-ups with these compiler flags on
# P4/Xeon systems:
#-O3 -DNDEBUG -fno-strict-aliasing -fomit-frame-pointer -fexpensive-optimizations -falign-functions=4 -funroll-loops -malign-double -fprefetch-loop-arrays -march=pentium4 -mcpu=pentium4 -msse2 -mfpmath=sse 

CXXFLAGS          += -DLINUX -Wall
CXX_RELEASE_FLAGS += -O3 -DNDEBUG -fno-strict-aliasing
CXX_DEBUG_FLAGS   += -g -Werror

CFLAGS            += -DLINUX -Wall -Wmissing-prototypes -Wsign-compare
C_RELEASE_FLAGS   += -O3 -DNDEBUG -fno-strict-aliasing
C_DEBUG_FLAGS     += -g -Werror

ifeq ($(MACHTYPE),x86_64)
ifeq ($(FORCE_32BIT_ABI),1)
LDFLAGS           += -m32 -L/usr/X11R6/lib
CFLAGS            += -m32 -fPIC
else
LDFLAGS           += -L/usr/X11R6/lib64
CFLAGS            += -fPIC
endif
else
LDFLAGS           += -L/usr/X11R6/lib
endif
LD_RELEASE_FLAGS  += 
LD_DEBUG_FLAGS    += 

ifeq ($(MACHTYPE), alpha)
CXXFLAGS          += -fPIC -mieee 
CFLAGS            += -fPIC -mieee
endif

ifeq ($(MACHTYPE), mips)
ifeq ($(shell ls /proc/ps2pad), /proc/ps2pad)
PLAYSTATION2      =   1
CXXFLAGS          += -DPLAYSTATION2
CFLAGS            += -DPLAYSTATION2
endif
endif

ifeq ($(MACHTYPE), ia64)
CFLAGS            += -fPIC
endif

PROFILEFLAGS = -pg -a

CAT = cat
AS = as
LEX = flex -t
LEXLIB = -ll
YACC = bison -y -d
LD = $(CXX)
AR = ar
ARCREATEFLAGS = cr
RANLIB = true
LN = ln -s
MKDIR = mkdir -p
RM = rm -f
CP = cp
MAKE = gmake -s
NOWEB = noweb
LATEX = latex
BIBTEX = bibtex
DVIPS = dvips -t letter
GHOSTSCRIPT = gs
LIBPREFIX = lib
DLLSUFFIX = .so
LIBSUFFIX = .a
OBJSUFFIX = .o
MV = mv
SHARED_LDFLAGS += -shared 
PERL = perl
PYTHON = python
JGRAPH = /u/eldridge/bin/IRIX/jgraph
PS2TIFF = pstotiff.pl
PS2TIFFOPTIONS = -alpha -mag 2
PS2PDF = ps2pdf

MPI_CC = mpicc
MPI_CXX = mpiCC
MPI_LDFLAGS =

QT=0
ifeq ($(QT),1)
    QTDIR=/insert/path/to/qt/here/qt-2.3.1
    MOC=$(QTDIR)/bin/moc
    UIC=$(QTDIR)/bin/uic
endif

ifeq ($(USE_DMX), 1)
CXXFLAGS += -DUSE_DMX
CFLAGS += -DUSE_DMX
endif

