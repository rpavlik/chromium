# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

# kept getting included twice
ifndef DARWIN

DARWIN = 1
INSTALL_DIR = /cr

G++-INCLUDE-DIR = /usr/include/g++
CXX = g++ -fno-common
CC = gcc -fno-common

CXXFLAGS          += -DDARWIN -Wall -Wno-format
CXX_RELEASE_FLAGS += -O3 -DNDEBUG
CXX_DEBUG_FLAGS   += -g

CFLAGS            += -DDARWIN -Wall -Wno-format
C_RELEASE_FLAGS   += -O3 -DNDEBUG
C_DEBUG_FLAGS     += -g

LDFLAGS           += 
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
LIBSUFFIX = .a
OBJSUFFIX = .o
MV = mv
PERL = perl
PYTHON = python
JGRAPH = /u/eldridge/bin/IRIX/jgraph
PS2TIFF = pstotiff.pl
PS2TIFFOPTIONS = -alpha -mag 2
PS2PDF = ps2pdf

MPI_CC = mpicc
MPI_CXX = mpiCC
MPI_LDFLAGS =

ifdef SPU
LIBPREFIX = 
DLLSUFFIX = .bundle
SHARED_LDFLAGS = -bundle
else
LIBPREFIX = lib
DLLSUFFIX = .dylib
SHARED_LDFLAGS = -dynamiclib -multiply_defined suppress
endif

GLUT_INC   = /System/Library/Frameworks/GLUT.framework/Headers
OPENGL_INC = /System/Library/Frameworks/OpenGL.framework/Headers

QT=0
ifeq ($(QT),1)
    QTDIR=/insert/path/to/qt/here/qt-2.3.1
    MOC=$(QTDIR)/bin/moc
    UIC=$(QTDIR)/bin/uic
endif

endif
