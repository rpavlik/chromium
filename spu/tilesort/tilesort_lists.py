# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

import sys

sys.path.append( "../../glapi_parser" )
import apiutil


keys = apiutil.GetDispatchedFunctions("../../glapi_parser/APIspec.txt")

apiutil.CopyrightC()

print """#include <stdio.h>
#include "cr_spu.h"
#include "tilesortspu.h"
#include "tilesortspu_proto.h"
#include "cr_packfunctions.h"
#include "cr_glstate.h"


static int TILESORTSPU_APIENTRY tilesortspu_nop(void)
{
  return 0;
}


#define CHANGE( name, func ) crSPUChangeInterface( (void *)&(tilesort_spu.self), (void *)tilesort_spu.self.name, (void *)((SPUGenericFunction) func) )
#define CHANGESWAP( name, swapfunc, regfunc ) crSPUChangeInterface( (void *)&(tilesort_spu.self), (void *)tilesort_spu.self.name, (void *)((SPUGenericFunction) (tilesort_spu.swap ? swapfunc: regfunc )) )

"""


print "void tilesortspuLoadListTable(void)"
print "{"
# XXX NOTE: this should basically be identical to the tilesort.py code.
for func_name in keys:
	if apiutil.FindSpecial( "tilesort_unimplemented", func_name ):
		continue
	elif apiutil.FindSpecial( "tilesort_list", func_name ):
		continue
	elif apiutil.FindSpecial( "tilesort_bbox", func_name ):
                print '\tCHANGESWAP( %s, crPack%sBBOX_COUNTSWAP, crPack%sBBOX_COUNT );' % (func_name, func_name, func_name)
	else:
		 print '\tCHANGESWAP( %s, crPack%sSWAP, crPack%s );' % (func_name, func_name, func_name )

print "}"


print ""
print ""
print "void tilesortspuLoadSortTable(void)"
print "{"
# XXX NOTE: this should basically be identical to the tilesort.py code.
for func_name in keys:
	if apiutil.FindSpecial( "tilesort_list", func_name ):
		continue
	if apiutil.FindSpecial( "tilesort", func_name ):
		print '\tCHANGE( %s, tilesortspu_%s );' % (func_name, func_name )
	elif apiutil.FindSpecial( "tilesort_unimplemented", func_name ):
		print '\tCHANGE( %s, tilesortspu_%s );' % (func_name, func_name )
	elif apiutil.FindSpecial( "tilesort_state", func_name ):
		print '\tCHANGE( %s, crState%s );' % (func_name, func_name )
	elif apiutil.FindSpecial( "tilesort_bbox", func_name ):
                print '\tCHANGESWAP( %s, crPack%sBBOX_COUNTSWAP, crPack%sBBOX_COUNT );' % (func_name, func_name, func_name)
	else:
		 print '\tCHANGESWAP( %s, crPack%sSWAP, crPack%s );' % (func_name, func_name, func_name )

print "}"


print ""
print ""
print "/* Used when playing back display lists locally to update state */"
print "void tilesortspuLoadStateTable(SPUDispatchTable *t)"
print "{"
for func_name in keys:
	props = apiutil.Properties(func_name)

	if (func_name == "CallList" or
		func_name == "CallLists"):
		print '\tt->%s = tilesortspuState%s;' % (func_name, func_name)
	elif func_name == "Bitmap":
		print '\tt->%s = crState%s;' % (func_name, func_name)
	elif func_name == "NewList":
		print '\tt->%s = tilesortspuState%s;' % (func_name, func_name)
	elif func_name == "EndList":
		print '\tt->%s = tilesortspuState%s;' % (func_name, func_name)
	elif func_name == "PopAttrib":
		print '\tt->%s = tilesortspu_%s;' % (func_name, func_name)
	elif ("get" in props or
		  "nolist" in props or
		  func_name == "NewList" or
		  func_name == "EndList" or
		  func_name == "IsList" or
		  func_name == "GenLists" or
		  func_name == "DeleteLists" or
		  func_name == "DeleteTextures" or
		  func_name == "DeleteProgramsARB" or
		  func_name == "DeleteProgramsNV" or
		  func_name == "RenderMode" or
		  func_name == "FeedbackBuffer" or
		  func_name == "SelectionBuffer" or
		  func_name == "Finish" or
		  func_name == "Flush"):
		# do nothing - these'll never be in a display list anyway.
		pass
	else:
		if apiutil.FindSpecial( "tilesort", func_name ):
			print '\tt->%s = (%sFunc_t) tilesortspu_nop;' % (func_name, func_name)
		elif apiutil.FindSpecial( "tilesort_unimplemented", func_name ):
			print '\tt->%s = (%sFunc_t) tilesortspu_nop;' % (func_name, func_name)
		elif apiutil.FindSpecial( "tilesort_state", func_name ):
			print '\tt->%s = crState%s;' % (func_name, func_name)
		elif apiutil.FindSpecial( "tilesort_bbox", func_name ):
			print '\tt->%s = (%sFunc_t) tilesortspu_nop;' % (func_name, func_name)
		else:
			print '\tt->%s = (%sFunc_t) tilesortspu_nop;' % (func_name, func_name)
print "}"


print ""
print ""
print "/* Used to send display lists to servers */"
print "void tilesortspuLoadPackTable( SPUDispatchTable *t )"
print "{"
for func_name in keys:
	props = apiutil.Properties(func_name)
	
	if (func_name == "CallList" or
		func_name == "CallLists" or
		func_name == "Clear" or
		func_name == "Accum" or
		func_name == "Begin" or
		func_name == "End" or
		func_name == "BoundsInfoCR" or
		func_name == "PopAttrib"):
		print '\tt->%s = tilesort_spu.swap ? crPack%sSWAP : crPack%s;' % (func_name, func_name, func_name)
	elif (func_name == "Bitmap" or
		  func_name == "DrawPixels" or
		  func_name == "ZPix" or
		  func_name == "TexImage1D" or
		  func_name == "TexImage2D" or
		  func_name == "TexImage3D" or
		  func_name == "TexImage3DEXT" or
		  func_name == "TexSubImage1D" or
		  func_name == "TexSubImage2D" or
		  func_name == "TexSubImage3D"):
		print '\tt->%s = tilesortspu_Pack%s;' % (func_name, func_name)
	elif apiutil.FindSpecial("tilesort", func_name):
		print '\tt->%s = tilesortspu_%s;' % (func_name, func_name)
	elif (("setclient" in props or "get" in props) and
		  func_name != "RenderMode"):
		print '\tt->%s = crState%s;' % (func_name, func_name)
	elif not "nolist" in props:
		print '\tt->%s = tilesort_spu.swap ? crPack%sSWAP : crPack%s;' % (func_name, func_name, func_name)
	else:
		print '\tt->%s = (%sFunc_t) tilesortspu_nop;' % (func_name, func_name)
print "}"

