# cpg - 4/08/02

G++-INCLUDE-DIR = /usr/include/g++
CXX = cxx
CC = cc

CXXFLAGS          += -DOSF1 -Wall -Werror -pthread
CXX_RELEASE_FLAGS += -g -O3 -DNDEBUG
CXX_DEBUG_FLAGS   += -g

CFLAGS            += -DOSF1 -Wall -Werror -pthread
C_RELEASE_FLAGS   += -g -O3 -DNDEBUG
C_DEBUG_FLAGS     += -g

LDFLAGS           += -L/usr/X11R6/lib
LD_RELEASE_FLAGS  += 
LD_DEBUG_FLAGS    += 

ifeq ($(MACHTYPE), alpha)
CXXFLAGS          += -mieee -mcpu=ev67
CFLAGS            += -mieee -mcpu=ev67
endif

PROFILEFLAGS = -pg -a

MATH = 1

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
MAKE = make -s
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

