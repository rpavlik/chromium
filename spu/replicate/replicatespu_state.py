# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

import sys

sys.path.append( "../../glapi_parser" )
import apiutil


keys = apiutil.GetDispatchedFunctions("../../glapi_parser/APIspec.txt")


apiutil.CopyrightC()

print """
#include <stdio.h>
#include "cr_server.h"
#include "cr_packfunctions.h"
#include "replicatespu.h"
"""

for func_name in keys:
	return_type = apiutil.ReturnType(func_name)
	params = apiutil.Parameters(func_name)
	if (apiutil.FindSpecial( "replicatespu_state", func_name ) or apiutil.FindSpecial( "replicatespu_get", func_name)):
		print 'extern %s REPLICATESPU_APIENTRY replicatespu_%s( %s );' % ( return_type, func_name, apiutil.MakeDeclarationString( params ) )

for func_name in keys:
	return_type = apiutil.ReturnType(func_name)
	params = apiutil.Parameters(func_name)
	if (apiutil.FindSpecial( "replicatespu_state", func_name ) or apiutil.FindSpecial( "replicatespu_get", func_name)):
		print '%s REPLICATESPU_APIENTRY replicatespu_%s( %s )' % ( return_type, func_name, apiutil.MakeDeclarationString( params ) )
		print '{'
		if apiutil.FindSpecial( "replicatespu_state", func_name ):
			print '\tif (replicate_spu.swap)'
			print '\t{'
			print '\t\tcrPack%sSWAP( %s );' % (func_name, apiutil.MakeCallString( params ) )
			print '\t}'
			print '\telse'
			print '\t{'
			print '\t\tcrPack%s( %s );' % (func_name, apiutil.MakeCallString( params ) )
			print '\t}'
		print ''
		if apiutil.FindSpecial( "replicatespu_get", func_name ):
			print '\treturn crState%s( %s );' % ( func_name, apiutil.MakeCallString( params ) )
		else:
			print '\tcrState%s( %s );' % ( func_name, apiutil.MakeCallString( params ) )
		print '}'
