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
#include "tilesortspu.h"
#include "tilesortspu_proto.h"
#include "cr_error.h"    
"""

for func_name in keys:
	if apiutil.FindSpecial( "tilesort_unimplemented", func_name ):
		return_type = apiutil.ReturnType(func_name)
		params = apiutil.Parameters(func_name)
		print '%s TILESORTSPU_APIENTRY tilesortspu_%s( %s )' % ( return_type, func_name, apiutil.MakeDeclarationString(params) )	
		print '{'
		for (name, type, vecSize) in params:
			print '\t(void) %s;' % name
		print ''
		print '\tcrWarning("Unimplemented tilesort function %s\\n");' % func_name
		if return_type != 'void':
			print ''
			print '\treturn 0;'
		print '}'
		print ''
