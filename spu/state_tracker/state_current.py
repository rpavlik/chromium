# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

import sys;

sys.path.append( '../../packer' )
sys.path.append( '../../opengl_stub' )
from pack_currenttypes import *;
import stub_common;

stub_common.CopyrightC()

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
			
			if (ucname == 'COLOR' or ucname == 'NORMAL' or ucname == 'SECONDARYCOLOR') and type != 'f' and type != 'd':
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
	static const GLcolorf color_default			= {0.0f, 0.0f, 0.0f, 1.0f};
	static const GLcolorf secondaryColor_default= {0.0f, 0.0f, 0.0f, 0.0f};
	static const GLtexcoordf texCoord_default	= {0.0f, 0.0f, 0.0f, 1.0f};
	static const GLvectorf normal_default		= {0.0f, 0.0f, 0.0f, 1.0f};
	static const GLfloat index_default			= 0.0f;
	static const GLboolean edgeFlag_default		= GL_TRUE;
	GLnormal_p		*normal		= &(c->current->normal);
	GLcolor_p		*color		= &(c->current->color);
	GLsecondarycolor_p *secondaryColor = &(c->current->secondaryColor);
	GLtexcoord_p	*texCoord	= &(c->current->texCoord);
	GLindex_p		*index		= &(c->current->index);
	GLedgeflag_p	*edgeFlag	= &(c->current->edgeFlag);
	int i;
	GLbitvalue nbitID[CR_MAX_BITARRAY];

	DIRTY(nbitID, g->neg_bitid);

	/* Save pre state */
	c->normalPre = c->normal;
	c->colorPre = c->color;
	c->secondaryColorPre = c->secondaryColor;
	for (i = 0 ; i < CR_MAX_TEXTURE_UNITS ; i++)
	{
		c->texCoordPre[i] = c->texCoord[i];
	}
	c->indexPre = c->index;
	c->edgeFlagPre = c->edgeFlag;

'''

for k in current_fns.keys():
	print '\t/* %s */' % k
	print '\tv=NULL;'
	name = '%s%s' % (k[:1].lower(),k[1:])

	indent = ""
	if current_fns[k].has_key( 'array' ):
		print '\tfor (i = 0 ; i < %s ; i++)' % current_fns[k]['array']
		print '\t{'
		indent += "\t"
	for type in current_fns[k]['types']:
		for size in current_fns[k]['sizes']:
			ptr = '%s->%s%d' % (name, type, size )
			if current_fns[k].has_key( 'array' ):
				ptr += "[i]"
			print '%s\tif (v < %s)' % (indent, ptr)
			print '%s\t{' % indent
			print '%s\t\tv = %s;' % (indent, ptr)
			if (k == 'Color' or k == 'Normal' or k == 'SecondaryColor') and type != 'f' and type != 'd' and type != 'l':
				print '%s\t\tconvert = (convert_func) __convert_rescale_%s%d;' % (indent,type,size)
			else:
				print '%s\t\tconvert = (convert_func) __convert_%s%d;' % (indent,type,size)
			print '%s\t}' % indent
	print ''
	print '%s\tif (v != NULL) {' % indent
	if current_fns[k].has_key( 'array' ):
		print '%s\t\tc->%s[i] = %s_default;' % (indent,name,name)
	else:
		print '%s\t\tc->%s = %s_default;' % (indent,name,name)
	if k == 'EdgeFlag':
		print '%s\t\t__convert_boolean (&(c->%s), v);' % (indent, name)
	elif k == 'Index':
		print '%s\t\tconvert(&(c->%s), v);' % (indent,name)
	elif k == 'Normal':
		print '%s\t\tconvert(&(c->%s.x), v);' % (indent,name)
	elif k == 'TexCoord':
		print '%s\t\tconvert(&(c->%s[i].s), v);' % (indent,name)
	elif k == 'Color':
		print '%s\t\tconvert(&(c->%s.r), v);' % (indent,name)
	elif k == 'SecondaryColor':
		print '%s\t\tconvert(&(c->%s.r), v);' % (indent,name)
	if current_fns[k].has_key( 'array' ):
		print '%s\t\tDIRTY(cb->%s[i], nbitID);' % (indent,name)
	else:
		print '%s\t\tDIRTY(cb->%s, nbitID);' % (indent,name)
	print '%s\t\tDIRTY(cb->dirty, nbitID);' % indent
	print '%s\t}' % indent
	if current_fns[k].has_key( 'array' ):
		print '%s\t%s->ptr[i] = v;' % (indent, name )
	else:
		print '%s\t%s->ptr = v;' % (indent, name )
	if current_fns[k].has_key( 'array' ):
		print '\t}'
print '}'
