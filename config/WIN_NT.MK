# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

WINDOWS=1
G++-INCLUDE-DIR = /usr/include/g++
CXX = cl
CXXFLAGS += /nologo /DWIN32 /DWINDOWS /TP /W4 /WX /MD
CC = cl
CFLAGS += /nologo /DWIN32 /DWINDOWS /W4 /WX /MD /TC
LDFLAGS += /nologo
DEBUGFLAGS = /Yd /Z7 /GZ
RELEASEFLAGS = /Ox /DNDEBUG
CAT = cat
LEX = flex -t
LEXLIB = -ll
YACC = bison -y -d
LD = cl
LDFLAGS += 
AR = ar
ARCREATEFLAGS = cr
RANLIB = true
LN = ln -s
MKDIR = mkdir -p
ECHO := echo
CLEAR := cls
RM = rm -f
CP = cp
MV = mv
MAKE = make --no-print-directory
NOWEB = noweb
LATEX = latex
BIBTEX = bibtex
DVIPS = dvips -t letter
GHOSTSCRIPT = gs
LIBPREFIX = 
DLLSUFFIX = .dll
LIBSUFFIX = .lib
OBJSUFFIX = .obj
EXESUFFIX = .exe
SHARED_LDFLAGS = /LDd
PERL = perl
PYTHON = python.exe
JGRAPH = jgraph
MPI_CC = cl
MPI_CXX = cl
MPI_LDFLAGS += mpichd.lib
