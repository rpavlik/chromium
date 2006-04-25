# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.


# This file lets one set various compile-time options.


# Set RELEASE to 1 to compile with optimizations and without debug info.
RELEASE=0


# Set THREADSAFE to 1 if you want thread safety for parallel applications.
THREADSAFE=0


# Set to 1 if you want to force building 32-bit objects on a 64-bit system.
FORCE_32BIT_ABI=0

# Set USE_DMX to 1 if you want to enable DMX support.
# You'll need the libdmx.a library and dmxext.h header in the usual X
# directories.  If you don't, set DMX_INCDIR and DMX_LIBDIR here.
USE_DMX=0
#DMX_DIR=/wherever/DMX/is/installed
#DMX_INCDIR=${DMX_DIR}/dmx/xc/exports/include/
#DMX_LIBDIR=${DMX_DIR}/dmx/xc/exports/lib/

# Set USE_REPLICATE to 1 if you want to enable the "replicate" SPU build,
# which can be used with a specially-constructed VNC viewer to use local
# acccelerated rendering to display windows belonging to OpenGL applications.
# You'll need the libVncExt.so.2.0 library and vncstr.h & vnc.h headers in
# the usual X directories.
USE_REPLICATE=1

# Set USE_VNC to 1 if you want to enable the "vnc" SPU build.
# You'll need the libVncExt.so.2.0 library and vncstr.h & vnc.h &
# Xcliplist.h headers in the usual X directories.
USE_VNC=1

# Set USE_OSMESA to 1 if you want to enable off screen rendering using Mesa.
USE_OSMESA=0


# Quadrics Elan3 interface support.  There are two different implementations
# of Quadrics communication.  TEAC_SUPPORT is probably what you want;
# TCSCOMM_SUPPORT is somewhat outdated at this point.
TEAC_SUPPORT=0
TCSCOMM_SUPPORT=0


# Set to 1 to enable Myrinet support
GM_SUPPORT=0


# Set to 1 to enable InfiniBand Support
IB_SUPPORT=0

# Set to 1 to enable SDP Support
SDP_SUPPORT=0

# Set to 1 to enable DOXYGEN generation
DOXYGEN=0

# Set to 1 to enable (limited) NetLogger support
NETLOGGER=0
# Set NLHOME to NetLogger install dir (i.e. /usr/local)
NLHOME=
