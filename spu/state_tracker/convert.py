# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

import sys;

sys.path.append( '../../packer' )
sys.path.append( '../../opengl_stub' )
from pack_currenttypes import *;
import stub_common

stub_common.CopyrightC()

print '''
#include "state/cr_statetypes.h"
#include "cr_glwrapper.h"

static double __read_double( void *src )
{
    unsigned int *ui = (unsigned int *) src;
    double d;
    ((unsigned int *) &d)[0] = ui[0];
    ((unsigned int *) &d)[1] = ui[1];
    return d;
}
'''

for k in gltypes.keys():
	for i in range(1,5):
		print 'void __convert_%s%d (GLfloat *dst, %s *src) {' % (k,i,gltypes[k]['type'])
		if k == 'd':
		  for j in range(i-1):
		    print '\t*dst++ = (GLfloat) __read_double(src++);'
		  print '\t*dst = (GLfloat) __read_double(src);'
		else:
		  for j in range(i-1):
		    print '\t*dst++ = (GLfloat) *src++;';
		  print '\t*dst = (GLfloat) *src;';
		print '}\n';

scale = {
	'ub' : 'GL_MAXUBYTE',
	'b'  : 'GL_MAXBYTE',
	'us' : 'GL_MAXUSHORT',
	's'  : 'GL_MAXSHORT',
	'ui' : 'GL_MAXUINT',
	'i'  : 'GL_MAXINT',
	'f'  : '',
	'd'  : ''
}

for k in gltypes.keys():
	if k != 'f' and k != 'd' and k != 'l':
		for i in range(1,5):
			print 'void __convert_rescale_%s%d (GLfloat *dst, %s *src) {' % (k,i,gltypes[k]['type'])
			for j in range(i-1):
				print '\t*dst++ = ((GLfloat) *src++) / %s;' % scale[k]
			print '\t*dst = ((GLfloat) *src) / %s;' % scale[k]
			print '}\n'

print '''

void __convert_boolean (GLboolean *dst, GLboolean *src) {
	*dst = *src;
}
'''
