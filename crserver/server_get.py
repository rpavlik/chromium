import sys,os;
import cPickle;
import string;
import re;

sys.path.append( "../opengl_stub" )
parsed_file = open( "../glapi_parser/gl_header.parsed", "rb" )
gl_mapping = cPickle.load( parsed_file )

import stub_common;

print """
#include "cr_spu.h"
#include "cr_glwrapper.h"
#include "cr_error.h"
#include "cr_mem.h"
#include "cr_net.h"
#include "server_dispatch.h"
#include "server.h"
"""

keys = gl_mapping.keys()
keys.sort()

max_components = {
	'GetClipPlane': 4,
	'GetLightfv': 4,
	'GetLightiv': 4,
	'GetMaterialfv': 4, 
	'GetMaterialiv': 4, 
	'GetPolygonStipple': 32*32/8,
	'GetTexEnvfv': 4,
	'GetTexEnviv': 4,
	'GetTexGendv': 4,
	'GetTexGenfv': 4,
	'GetTexGeniv': 4,
	'GetTexLevelParameterfv': 1,
	'GetTexLevelParameteriv': 1,
	'GetTexParameterfv': 4,
	'GetTexParameteriv': 4,
}

no_pnames = [ 'GetClipPlane', 'GetPolygonStipple', 'GetTexLevelParameterfv', 'GetTexLevelParameteriv' ];

num_components = {
	'GL_AMBIENT' : 4, 
	'GL_DIFFUSE' : 4,
	'GL_SPECULAR' : 4,
	'GL_POSITION' : 4,
	'GL_SPOT_DIRECTION' : 3,
	'GL_SPOT_EXPONENT' : 1, 
	'GL_SPOT_CUTOFF' : 1, 
	'GL_CONSTANT_ATTENUATION' : 1, 
	'GL_LINEAR_ATTENUATION' : 1, 
	'GL_QUADRATIC_ATTENUATION' : 1, 
	'GL_EMISSION' : 4, 
	'GL_SHININESS' : 1, 
	'GL_COLOR_INDEXES' : 3, 
	'GL_TEXTURE_ENV_MODE' : 1,
	'GL_TEXTURE_ENV_COLOR' : 4, 
	'GL_TEXTURE_GEN_MODE' : 1, 
	'GL_OBJECT_PLANE' : 4, 
	'GL_EYE_PLANE' : 4, 
	'GL_TEXTURE_MAG_FILTER' : 1,
	'GL_TEXTURE_MIN_FILTER' : 1, 
	'GL_TEXTURE_WRAP_S' : 1, 
	'GL_TEXTURE_WRAP_T' : 1, 
	'GL_TEXTURE_BORDER_COLOR' : 4
}

print """unsigned int LookupComponents( GLenum pname )
{
	switch( pname )
	{
"""
comps = num_components.keys();
comps.sort();
for comp in comps:
	print '\t\t\tcase %s: return %d;' % (comp,num_components[comp])

print """
		default:
			crError( "Unknown paramater name in LookupComponents: %d", pname );
			break;
	}
	// NOTREACHED
	return 0;
}
"""

for func_name in keys:
	(return_type, arg_names, arg_types) = gl_mapping[func_name]
	if stub_common.FindSpecial( "../packer/packer_get", func_name ) and not stub_common.FindSpecial( "server", func_name ):
		print 'void SERVER_DISPATCH_APIENTRY crServerDispatch%s%s' % (func_name, stub_common.ArgumentString( arg_names, arg_types ) )
		print '{'
		local_argtype = string.replace( arg_types[len(arg_types)-1], '*', '' )
		local_argname = 'local_%s' % arg_names[len(arg_names)-1]
		print '\t%s %s[%d];' % ( local_argtype, local_argname, max_components[func_name] )
		print '\t(void) %s;' % arg_names[len(arg_names)-1]
		arg_names[len(arg_names)-1] = local_argname
		print '\tcr_server.head_spu->dispatch_table.%s%s;' % ( func_name, stub_common.CallString(arg_names) )
		if func_name in no_pnames:
			print '\tcrServerReturnValue( &(%s[0]), %d*sizeof(%s) );' % (local_argname, max_components[func_name], local_argtype );
		else:
			print '\tcrServerReturnValue( &(%s[0]), LookupComponents(pname)*sizeof(%s) );' % (local_argname, local_argtype );
		print '}\n'
