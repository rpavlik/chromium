import sys;

sys.path.append( '../../packer' )
from pack_currenttypes import *;

print '''
#include "state/cr_currentpointers.h"
#include "cr_glstate.h"

#include <stdio.h>

typedef void (*convert_func) (GLfloat *, unsigned char *);
'''

import convert

for k in current_fns.keys():
	name = k
	name = '%s%s' % (k[:1].lower(),k[1:])
	ucname = k.upper()
	num_members = len(current_fns[k]['default']) + 1

	print '#define VPINCH_CONVERT_%s(op,data,dst) \\' % ucname
	print '{\\'
	print '\tGLfloat vdata[%d] = {' % num_members,

## Copy dst data into vdata
	i = 0;
	for defaultvar in current_fns[k]['default']:
		print '%d' % defaultvar,
		if i != num_members:
			print ',',
		i += 1
	print '};\\'

	print '\tswitch (op) { \\'
	for type in current_fns[k]['types']:
		for size in current_fns[k]['sizes']:
			uctype = type.upper()
			if ucname == 'EDGEFLAG':
				print '\tcase CR_%s_OPCODE: \\' % ucname
			else:
				print '\tcase CR_%s%d%s_OPCODE: \\' % (ucname,size,uctype)
			
			if (ucname == 'COLOR' or ucname == 'NORMAL') and type != 'f' and type != 'd':
				print '\t\t__convert_rescale_%s%d (vdata, (%s *) (data)); \\' % (type,size,gltypes[type]['type'])
			else:
				print '\t\t__convert_%s%d (vdata, (%s *) (data)); \\' % (type,size,gltypes[type]['type'])
			print '\t\tbreak; \\'

	print '\tdefault: \\'
	print '\t\tcrSimpleError ( "Unknown opcode in VPINCH_CONVERT_%s" ); \\' % ucname
	print '\t}\\'

	i = 0
	for member in current_fns[k]['members']:
		print '\t(dst).%s = vdata[%d];\\' % (member,i)
		i += 1

	print '}\n'

print '''

void crStateCurrentRecover( void )
{
	unsigned char *v;
	convert_func convert=NULL;
	CRContext *g = GetCurrentContext();
	CRCurrentState *c = &(g->current);
	CRStateBits *b = GetCurrentBits();
	CRCurrentBits *cb = &(b->current);
	GLbitvalue nbitID = g->neg_bitid;
	static const GLcolorf color_default			= {0.0f, 0.0f, 0.0f, 1.0f};
	static const GLtexcoordf texCoord_default	= {0.0f, 0.0f, 0.0f, 1.0f};
	static const GLvectorf normal_default		= {0.0f, 0.0f, 0.0f, 1.0f};
	static const GLfloat index_default			= 0.0f;
	static const GLboolean edgeFlag_default		= GL_TRUE;
	GLnormal_p		*normal		= &(c->current->normal);
	GLcolor_p		*color		= &(c->current->color);
	GLtexcoord_p	*texCoord	= &(c->current->texCoord);
	GLindex_p		*index		= &(c->current->index);
	GLedgeflag_p	*edgeFlag	= &(c->current->edgeFlag);

	/* Save pre state */
	c->normalPre = c->normal;
	c->colorPre = c->color;
	c->texCoordPre = c->texCoord;
	c->indexPre = c->index;
	c->edgeFlagPre = c->edgeFlag;

'''

for k in current_fns.keys():
	print '\t/* %s */' % k
	print '\tv=NULL;'
	name = '%s%s' % (k[:1].lower(),k[1:])

	for type in current_fns[k]['types']:
		for size in current_fns[k]['sizes']:
			ptr = '%s->%s%d' % (name, type, size )
			print '\tif (v < %s)' % ptr
			print '\t{'
			print '\t\tv = %s;' % ptr
			if (k == 'Color' or k == 'Normal') and type != 'f' and type != 'd' and type != 'l':
				print '\t\tconvert = (convert_func) __convert_rescale_%s%d;' % (type,size)
			else:
				print '\t\tconvert = (convert_func) __convert_%s%d;' % (type,size)
			print '\t}'
	print ''
	print '\tif (v != NULL) {'
	print '\t\tc->%s = %s_default;' % (name,name)
	if k == 'EdgeFlag':
		print '\t\t__convert_boolean (&(c->%s), v);' % name
	elif k == 'Index':
		print '\t\tconvert(&(c->%s), v);' % name
	elif k == 'Normal':
		print '\t\tconvert(&(c->%s.x), v);' % name
	elif k == 'TexCoord':
		print '\t\tconvert(&(c->%s.s), v);' % name
	elif k == 'Color':
		print '\t\tconvert(&(c->%s.r), v);' % name
	print '\t\tcb->%s = nbitID;' % name
	print '\t\tcb->dirty = nbitID;'
	print '\t}'
	print '\t%s->ptr = v;' % name
print '}'
