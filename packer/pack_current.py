from pack_currenttypes import *;

print """
#include "packer.h"
#include "state/cr_currentpointers.h"

#include <stdio.h>

void crPackOffsetCurrentPointers( int offset )
{
	GLnormal_p		*normal		= &(cr_packer_globals.current.normal);
	GLcolor_p		*color		= &(cr_packer_globals.current.color);
	GLtexcoord_p	*texCoord	= &(cr_packer_globals.current.texCoord);
	GLindex_p		*index		= &(cr_packer_globals.current.index);
	GLedgeflag_p	*edgeFlag	= &(cr_packer_globals.current.edgeFlag);

"""

for k in current_fns.keys():
	name = '%s%s' % (k[:1].lower(),k[1:])
	for type in current_fns[k]['types']:
		for size in current_fns[k]['sizes']:
			ptr = "%s->%s%d" % (name, type, size )
			print "\tif ( %s )" % ptr
			print "\t\t%s += offset;" % ptr
print """
}

void crPackNullCurrentPointers( void )
{
	GLnormal_p		*normal		= &(cr_packer_globals.current.normal);
	GLcolor_p		*color		= &(cr_packer_globals.current.color);
	GLtexcoord_p	*texCoord	= &(cr_packer_globals.current.texCoord);
	GLindex_p		*index		= &(cr_packer_globals.current.index);
	GLedgeflag_p	*edgeFlag	= &(cr_packer_globals.current.edgeFlag);
"""

for k in current_fns.keys():
	name = '%s%s' % (k[:1].lower(),k[1:])
	for type in current_fns[k]['types']:
		for size in current_fns[k]['sizes']:
			ptr = "%s->%s%d" % (name, type, size )
			print "\t%s = NULL;" % ptr

print "}"
