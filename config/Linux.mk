# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

# Set USE_DMX to 1 if you want to enable DMX support.
# You'll need the libdmx.a library and dmxext.h header in the usual X
# directories.
USE_DMX = 1


G++-INCLUDE-DIR = /usr/include/g++
CXX = g++
CC = gcc

CXXFLAGS          += -DLINUX -Wall -Werror
CXX_RELEASE_FLAGS += -O3 -DNDEBUG
CXX_DEBUG_FLAGS   += -g

CFLAGS            += -DLINUX -Wall -Werror -Wmissing-prototypes
C_RELEASE_FLAGS   += -O3 -DNDEBUG
C_DEBUG_FLAGS     += -g

LDFLAGS           += -L/usr/X11R6/lib
LD_RELEASE_FLAGS  += 
LD_DEBUG_FLAGS    += 

ifeq ($(MACHTYPE), alpha)
CXXFLAGS          += -mieee 
CFLAGS            += -mieee
endif

ifeq ($(MACHTYPE), mips)
ifeq ($(shell ls /proc/ps2pad), /proc/ps2pad)
PLAYSTATION2      =   1
CXXFLAGS          += -DPLAYSTATION2
CFLAGS            += -DPLAYSTATION2
endif
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

