# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

G++-INCLUDE-DIR = /usr/include/g++
CXX = g++
CC = gcc
BINUTIL_LINK_HACK=1

#
# You'll need to recompile GLUT and GLU libs with SUN_OGL_NO_VERTEX_MACROS set.
# Modify these to point to your newly compiled libraries if they're installed in say
# your home directory -- or comment these out if you are lucky enought to be able to 
# install your own libs.
#
OTHER_INCLUDES = -I/export/mira3/willjl/src/sparc_solaris/glut-3.7/include
OTHER_LD       = -L/export/mira3/willjl/mygl -L/export/mira3/willjl/src/sparc_solaris/glut-3.7/lib/glut

#
# Set SOLARIS_9_X_BUG if you're getting GLXBadCurrentWindow errors.  You shouldn't need to
# set this in Solaris 8. 
#
OTHER_FLAGS = -DSOLARIS_9_X_BUG

CXXFLAGS          += -DSunOS -DSUN_OGL_NO_VERTEX_MACROS -Wall -Werror -fPIC $(OTHER_INCLUDES) $(OTHER_FLAGS)  
CXX_RELEASE_FLAGS += -O3 -DNDEBUG
CXX_DEBUG_FLAGS   += -g

CFLAGS            += -DSunOS -DSUN_OGL_NO_VERTEX_MACROS -Wall -Werror -fPIC $(OTHER_INCLUDES) $(OTHER_FLAGS) 
C_RELEASE_FLAGS   += -O3 -DNDEBUG
C_DEBUG_FLAGS     += -g

LDFLAGS           +=  -L/usr/X11R6/lib $(OTHER_LD)
LD_RELEASE_FLAGS  += 
LD_DEBUG_FLAGS    += 

PROFILEFLAGS = -pg -a

CAT = cat
AS = as
LEX = flex -t
LEXLIB = -ll
YACC = bison -y -d
LD = gld
AR = gar
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

