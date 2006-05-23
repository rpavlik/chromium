# Copyright (c) 2004-2005, Tungsten Graphics, Inc.
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

# Set to 1 to enable (limited) NetLogger support
NETLOGGER ?= 0


TOP = ../..

include ${TOP}/options.mk


SPU = 1
SHARED = 1
LIBRARY = vncspu
FILES = vncspu \
	vncspu_config \
	vncspu_init \
	actions \
	active \
	async_io \
	client_io \
	control \
	d3des \
	decode_hextile \
	decode_tight \
	encode \
	encode_tight \
	fbs_files \
	host_connect \
	host_io \
	logging \
	main \
	region \
	region_more \
	rfblib \
	translate

CFLAGS += -DCHROMIUM -DRASTER_BOTTOM_TO_TOP=1 -DTIGHT_OPTIMIZATIONS

LDFLAGS += -lX11 -lVncExt
LDFLAGS += -lz -ljpeg -L/usr/X11R6/lib -lX11

CFLAGS += -DHAVE_XCLIPLIST_EXT -I/usr/X11R6/include 
LDFLAGS += -lXcliplist

ifeq ($(USE_VNC), 1)
CFLAGS += -DHAVE_VNC_EXT
LDFLAGS += -lVncExt
endif

ifeq ($(NETLOGGER), 1)
ifndef NLHOME
$(error "You must have NLHOME set if you turn on NETLOGGER compilation")
endif
CFLAGS += -DNETLOGGER -I$(NLHOME)/include
LDFLAGS += -L$(NLHOME)/lib -lnllite
endif

LIBRARIES = spuload crutil crmothership

PRECOMP =
SLOP = $(PRECOMP)

LIB_DEFS += vncspu.def
include ${TOP}/cr.mk

