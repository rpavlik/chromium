# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

# Disabled warnings:
# 1174: The function "f" was declared but never referenced. 
# 3201: The parameter "i" was never referenced.
# 1209: The controlling expression is constant.
# 1552: The variable "i" is set but never used.

# If you want 64 bit support, then uncomment the following line:
#IRIX_64BIT = 1

G++-INCLUDE-DIR = /usr/include/g++
CXX = CC
CC = cc

CXX_RELEASE_FLAGS += -O2 -DNDEBUG
CXX_DEBUG_FLAGS   += -g

ifdef IRIX_64BIT
CXXFLAGS          += -DIRIX_64BIT -64 -fullwarn -DIRIX -w2 -woff 1174,3201,1209,1552
CFLAGS            += -DIRIX_64BIT -64 -fullwarn -DIRIX -w2 -woff 1174,3201,1209,1552
LDFLAGS           += -64 -lm -ignore_unresolved
SHARED_LDFLAGS = -shared -64
else
CXXFLAGS          += -n32 -fullwarn -DIRIX -w2 -woff 1174,3201,1209,1552
CFLAGS            += -n32 -fullwarn -DIRIX -w2 -woff 1174,3201,1209,1552
LDFLAGS           += -n32 -lm -ignore_unresolved
SHARED_LDFLAGS = -shared -n32
endif
C_RELEASE_FLAGS   += -O2 -DNDEBUG
C_DEBUG_FLAGS     += -g

LD_RELEASE_FLAGS  += 
LD_DEBUG_FLAGS    +=

PROFILEFLAGS = -pg -a

CAT = cat
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
PARALLELMAKEFLAGS =
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
PERL = perl
PYTHON = python
JGRAPH = /u/eldridge/bin/IRIX/jgraph
PS2TIFF = pstotiff.pl
PS2TIFFOPTIONS = -alpha -mag 2
PS2PDF = ps2pdf

MPI_CC = cc
MPI_CXX = CC
MPI_LDFLAGS = -lmpi
SLOP += so_locations

QT=0
ifeq ($(QT),1)
    QTDIR=/insert/path/to/qt/here/qt-2.3.1
    MOC=$(QTDIR)/bin/moc
    UIC=$(QTDIR)/bin/uic
endif

