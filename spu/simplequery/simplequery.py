# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.


import sys

sys.path.append( "../../glapi_parser" )
import apiutil


apiutil.CopyrightC()

print """#include <stdio.h>
#include "cr_spu.h"
#include "cr_glstate.h"
#include "state/cr_stateerror.h"
#include "simplequeryspu.h"

"""

specials = [
	"WindowCreate",
	"CreateContext"
]

apiutil.GetAllFunctions("../../glapi_parser/APIspec.txt")
keys = apiutil.GetDispatchedFunctions()

print 'SPUNamedFunctionTable _cr_simplequery_table[] = {'
for func_name in keys:
	if not func_name in specials and "get" in apiutil.Properties(func_name):
		print '\t{ "%s", (SPUGenericFunction) crState%s },' % (func_name, func_name )
print "\t{ NULL, NULL }"
print "};"
