# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

import sys

sys.path.append( "../../glapi_parser" )
import apiutil


apiutil.CopyrightC()


print """
#ifndef HIDDENLINE_SPU_PROTO_H
#define HIDDENLINE_SPU_PROTO_H 1

"""

keys = apiutil.GetDispatchedFunctions("../../glapi_parser/APIspec.txt")

# Determine which functions to ignore
ignore_functions = []
for func_name in keys:
	if ("get" in apiutil.Properties(func_name) or
		"setclient" in apiutil.Properties(func_name) or
		"useclient" in apiutil.Properties(func_name) or
		apiutil.Category(func_name) == "Chromium" or
		apiutil.Category(func_name) == "GL_chromium"):
		ignore_functions.append(func_name)

specials = apiutil.AllSpecials( "hiddenline" ) + apiutil.AllSpecials( "hiddenline_pixel" )

# emit prototypes
for func_name in specials:
	return_type = apiutil.ReturnType(func_name)
	params = apiutil.Parameters(func_name)
	print 'extern %s HIDDENLINESPU_APIENTRY hiddenlinespu_%s(%s);' % (return_type, func_name, apiutil.MakeDeclarationString(params))


print """
#endif
"""
