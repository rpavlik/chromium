# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

import sys,os;
import cPickle;
import string;
import re;

sys.path.append( "../../opengl_stub" )
parsed_file = open( "../../glapi_parser/gl_header.parsed", "rb" )
gl_mapping = cPickle.load( parsed_file )

import stub_common;

keys = gl_mapping.keys()
keys.sort();

stub_common.CopyrightC()

print """#include <stdio.h>
#include "cr_spu.h"
#include "tilesortspu.h"
#include "cr_packfunctions.h"
#include "cr_glstate.h"
#include "cr_glwrapper.h"
"""

for func_name in keys:
	(return_type, args, types) = gl_mapping[func_name]
	if stub_common.FindSpecial( "tilesort_unimplemented", func_name ):
		continue
	if stub_common.FindSpecial( "tilesort", func_name ):
		print 'extern %s TILESORTSPU_APIENTRY tilesortspu_%s%s;' % ( return_type, func_name, stub_common.ArgumentString( args, types ) )

print """
#define CHANGE( name, func ) crSPUChangeInterface( &(tilesort_spu.self), tilesort_spu.self.name, (SPUGenericFunction) func )
#define CHANGESWAP( name, swapfunc, regfunc ) crSPUChangeInterface( &(tilesort_spu.self), tilesort_spu.self.name, (SPUGenericFunction) (tilesort_spu.swap ? swapfunc: regfunc ) )

static void __loadListAPI(void)
{
"""
for index in range(len(keys)):
	func_name = keys[index]
	(return_type, args, types) = gl_mapping[func_name]
	if stub_common.FindSpecial( "tilesort_unimplemented", func_name ):
		continue
	if stub_common.FindSpecial( "tilesort_list", func_name ):
		continue
	elif stub_common.FindSpecial( "tilesort_bbox", func_name ):
		print '\tCHANGESWAP( %s, crPack%sBBOX_COUNTSWAP, crPack%sBBOX_COUNT );' % (func_name, func_name, func_name)
	else:
		print '\tCHANGESWAP( %s, crPack%sSWAP, crPack%s );' % (func_name, func_name, func_name )
print """
}

static void __loadSortAPI(void)
{
"""

for index in range(len(keys)):
	func_name = keys[index]
	(return_type, args, types) = gl_mapping[func_name]
	if stub_common.FindSpecial( "tilesort_unimplemented", func_name ):
		continue
	if stub_common.FindSpecial( "tilesort_list", func_name ):
		continue
	if stub_common.FindSpecial( "tilesort", func_name ):
		print '\tCHANGE( %s, tilesortspu_%s );' % (func_name, func_name )
	elif stub_common.FindSpecial( "tilesort_state", func_name ):
		print '\tCHANGE( %s, crState%s );' % (func_name, func_name )
	elif stub_common.FindSpecial( "tilesort_bbox", func_name ):
		print '\tCHANGESWAP( %s, crPack%sBBOX_COUNTSWAP, crPack%sBBOX_COUNT );' % (func_name, func_name, func_name )
	else:
		print '\tCHANGESWAP( %s, crPack%sSWAP, crPack%s );' % (func_name, func_name, func_name )

print """
}

void TILESORTSPU_APIENTRY tilesortspu_NewList (GLuint list, GLuint mode) 
{
	/* the state tracker will do error checking and the flush for us. */
	crStateNewList( list, mode );
	if (tilesort_spu.swap)
	{
		crPackNewListSWAP( list, mode );
	}
	else
	{
		crPackNewList( list, mode );
	}

	__loadListAPI();
}

void TILESORTSPU_APIENTRY tilesortspu_EndList (void) 
{
	/* the state tracker will do error checking and the flush for us. */
	if (tilesort_spu.swap)
	{
		crPackEndListSWAP( );
	}
	else
	{
		crPackEndList( );
	}
	crStateEndList();

	__loadSortAPI();
	tilesortspuBroadcastGeom(0);

}
"""
