# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

DARWIN=1
G++-INCLUDE-DIR = /usr/include/g++
CXX = g++ -fno-common
CC = gcc -I/sw/include -I$(TOP)/include -I/usr/X11R6/include -fno-common

CXXFLAGS          += -DDARWIN -Wall -Werror -Wno-format
CXX_RELEASE_FLAGS += -O3 -DNDEBUG
CXX_DEBUG_FLAGS   += -g

CFLAGS            += -DDARWIN -Wall -Werror -Wno-format
C_RELEASE_FLAGS   += -O3 -DNDEBUG
C_DEBUG_FLAGS     += -g

LDFLAGS           += -L/usr/X11R6/lib -L/sw/lib
LD_RELEASE_FLAGS  += 
LD_DEBUG_FLAGS    += 

PROFILEFLAGS = -pg -a

CAT = cat
AS = as
LEX = flex -t
LEXLIB = -ll
YACC = bison -y -d
LD = $(CXX)
AR = ar
ARCREATEFLAGS = crs
RANLIB = ranlib
LN = ln -s
MKDIR = mkdir -p
RM = rm -f
CP = cp
MAKE = make
NOWEB = noweb
LATEX = latex
BIBTEX = bibtex
DVIPS = dvips -t letter
GHOSTSCRIPT = gs
LIBPREFIX = lib
DLLSUFFIX = .dylib
LIBSUFFIX = .a
OBJSUFFIX = .o
MV = mv
ifdef MOTHERSHIP
SHARED_LDFLAGS += -dynamiclib -noprebind
else
ifdef SPU
SHARED_LDFLAGS += -bundle -noprebind
else
SHARED_LDFLAGS += -dynamiclib -noprebind -multiply_defined suppress 
endif
endif

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

