# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.


# This file lets one set various compile-time options.


# Set RELEASE to 1 to compile with optimizations and without debug info.
RELEASE=0


# Set THREADSAFE to 1 if you want thread safety for parallel applications.
THREADSAFE=0


# Set USE_DMX to 1 if you want to enable DMX support.
# You'll need the libdmx.a library and dmxext.h header in the usual X
# directories.
USE_DMX=0

# Set USE_VNC to 1 if you want to enable VNC support.
# You'll need the libVncExt.so.2.0 library and vncstr.h & vnc.h headers in
# the usual X directories. This will build the replicateSPU.
# Note: you'll probably want to turn on threadsafety too (see below).
USE_VNC=0


# Set USE_OSMESA to 1 if you want to enable off screen rendering using Mesa.
USE_OSMESA=0
