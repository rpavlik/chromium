# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

# This script generates the pack_current.c file.

import sys
sys.path.append( "../opengl_stub" )
import stub_common

from pack_currenttypes import *;

stub_common.CopyrightC()

print """
/* DO NOT EDIT - THIS FILE GENERATED BY THE pack_current.py SCRIPT */

#include "packer.h"
#include "state/cr_currentpointers.h"

#include <stdio.h>

void crPackOffsetCurrentPointers( int offset )
{
	GET_PACKER_CONTEXT(pc);
	GLnormal_p		*normal		= &(pc->current.normal);
	GLcolor_p		*color		= &(pc->current.color);
	GLsecondarycolor_p	*secondaryColor	= &(pc->current.secondaryColor);
	GLtexcoord_p	*texCoord	= &(pc->current.texCoord);
	GLindex_p		*index		= &(pc->current.index);
	GLedgeflag_p	*edgeFlag	= &(pc->current.edgeFlag);
	GLvertexattrib_p *vertexAttrib = &(pc->current.vertexAttrib);
	GLfogcoord_p    *fogCoord   = &(pc->current.fogCoord);
	int i;
"""

for k in current_fns.keys():
	name = '%s%s' % (k[:1].lower(),k[1:])
	for type in current_fns[k]['types']:
		for size in current_fns[k]['sizes']:
			indent = ""
			ptr = "%s->%s%d" % (name, type, size )
			if current_fns[k].has_key( 'array' ):
				print '\tfor (i = 0 ; i < %s ; i++)' % current_fns[k]['array']
				print '\t{'
				ptr += "[i]"
				indent = "\t"
			print "%s\tif ( %s )" % (indent, ptr)
			print "%s\t{" % indent
			print "%s\t\t%s += offset;" % (indent, ptr )
			print "%s\t}" % indent
			if current_fns[k].has_key( 'array' ):
				print '\t}'
print """
}

void crPackNullCurrentPointers( void )
{
	GET_PACKER_CONTEXT(pc);
	GLnormal_p		*normal		= &(pc->current.normal);
	GLcolor_p		*color		= &(pc->current.color);
	GLsecondarycolor_p	*secondaryColor	= &(pc->current.secondaryColor);
	GLtexcoord_p	*texCoord	= &(pc->current.texCoord);
	GLindex_p		*index		= &(pc->current.index);
	GLedgeflag_p	*edgeFlag	= &(pc->current.edgeFlag);
	GLvertexattrib_p *vertexAttrib = &(pc->current.vertexAttrib);
	GLfogcoord_p    *fogCoord   = &(pc->current.fogCoord);
	int i;
"""

for k in current_fns.keys():
	name = '%s%s' % (k[:1].lower(),k[1:])
	for type in current_fns[k]['types']:
		for size in current_fns[k]['sizes']:
			ptr = "%s->%s%d" % (name, type, size )
			indent = ""
			if current_fns[k].has_key( 'array' ):
				print '\tfor (i = 0 ; i < %s ; i++)' % current_fns[k]['array']
				print '\t{'
				ptr += "[i]"
				indent = "\t"
			print "%s\t%s = NULL;" % (indent, ptr )
			if current_fns[k].has_key( 'array' ):
				print '\t}'

print "}"
