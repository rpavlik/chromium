G++-INCLUDE-DIR = /usr/include/g++
CXX = CC
CXXFLAGS += -n32
CC = cc
CFLAGS += -n32

FULLWARN = -fullwarn

DEBUGFLAGS = -g
RELEASEFLAGS = -O2 -DNDEBUG
PROFILEFLAGS = -pg -a

LDFLAGS += -n32 -lm
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
SHARED_LDFLAGS = -shared -n32
PERL = perl
PYTHON = python
JGRAPH = jgraph
PS2PDF = ps2pdf

MPI_CC = cc
MPI_CXX = CC
MPI_LDFLAGS = -lmpi
SLOP += so_locations
