# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

G++-INCLUDE-DIR = /usr/include/g++
CXX = g++
CC = gcc

CXXFLAGS          += -DLINUX -Wall -Werror
CXX_RELEASE_FLAGS += -g -O3 -DNDEBUG
CXX_DEBUG_FLAGS   += -g

CFLAGS            += -DLINUX -Wall -Werror
C_RELEASE_FLAGS   += -g -O3 -DNDEBUG
C_DEBUG_FLAGS     += -g

LDFLAGS           += -L/usr/X11R6/lib
LD_RELEASE_FLAGS  += 
LD_DEBUG_FLAGS    += 

ifeq ($(MACHTYPE), alpha)
CXXFLAGS          += -mieee
CFLAGS            += -mieee
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
