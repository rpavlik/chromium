# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

G++-INCLUDE-DIR = /usr/include/g++
CXX = c++ -fno-common
CXXFLAGS += -DDARWIN -Wall -Werror
CC = cc -I/sw/include -I/usr/X11R6/include -fno-common
CFLAGS += -DDARWIN -Wall -Werror

DEBUGFLAGS = -g
RELEASEFLAGS = -g -O3 -DNDEBUG
PROFILEFLAGS = -pg -a
CAT=cat

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
SHARED_LDFLAGS += -dynamiclib
LDFLAGS += -L/usr/X11R6/lib -L/sw/lib
PERL = perl
PYTHON = python
JGRAPH = /u/eldridge/bin/IRIX/jgraph
PS2TIFF = pstotiff.pl
PS2TIFFOPTIONS = -alpha -mag 2
PS2PDF = ps2pdf

MPI_CC = mpicc
MPI_CXX = mpiCC
MPI_LDFLAGS =
