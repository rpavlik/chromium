import sys,os;
import cPickle;
import string;
import re;

parsed_file = open( "../../glapi_parser/gl_header.parsed", "rb" )
gl_mapping = cPickle.load( parsed_file )

sys.path.append( "../../opengl_stub" )

import stub_common;

print """#include "cr_spu.h"
#include "cr_string.h"

#include <stdio.h>

static SPUGenericFunction __findFunc( char *name, SPU *spu )
{
	SPUNamedFunctionTable *temp;

	printf ("Finding %s\\n", name);

	if (spu == NULL)
		return NULL;

	for (temp = spu->function_table->table ; temp->name != NULL ; temp++)
	{
		if (!CRStrcmp( name, temp->name ) )
		{
			return temp->fn;
		}
	}
	printf ("Looking for %s in parent!\\n", name );
	return __findFunc( name, spu->superSPU );
}

void __buildDispatch( SPU *spu )
{
"""

keys = gl_mapping.keys()
keys.sort();

for func_name in keys:
	(return_type, names, types) = gl_mapping[func_name]
	print '\tspu->dispatch_table.%s = (%sFunc_t) __findFunc( "%s", spu );' % (func_name,func_name,func_name)
print '}'
