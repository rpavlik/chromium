G++-INCLUDE-DIR = /usr/include/g++
#CXX = g++
CXX=xlC
#CC = gcc
CC=xlc_r
CXXFLAGS += -DAIX  -I $(TOP)/include
CFLAGS += -qcpluscmt -DAIX -I $(TOP)/include 
LDFLAGS += -L/usr/X11R6/lib


DEBUGFLAGS = -g
RELEASEFLAGS = -DNDEBUG
PROFILEFLAGS = -pg -a

AS = as
LEX = flex -t
LEXLIB = -ll
#YACC = bison -y -d
YACC = yacc
LD = $(CXX)
AR = ar
ARCREATEFLAGS = cr
RANLIB = true
LN = ln -s
MKDIR = mkdir -p
RM = rm -f
CP = cp
MAKE = /scratch/gnu/bin/make -s
NOWEB = noweb
LATEX = latex
BIBTEX = bibtex
DVIPS = dvips -t letter
GHOSTSCRIPT = gs
LIBPREFIX = lib
DLLSUFFIX = .a
LIBSUFFIX = .a
OBJSUFFIX = .o
MV = mv
PERL = perl
PYTHON = python
JGRAPH = /u/eldridge/bin/IRIX/jgraph
PS2TIFF = pstotiff.pl
PS2TIFFOPTIONS = -alpha -mag 2
PS2PDF = ps2pdf

CAT = cat
MPI_CC = mpicc
MPI_CXX = mpiCC
MPI_LDFLAGS =
AIXSHAREDLIB=y
# SHARED=y
SHARED_LDFLAGS = -L $(TOP)/lib/$(ARCH) -lX11 -lXmu
ifdef OPENGL
    SHARED_LDFLAGS += -lGL
endif
