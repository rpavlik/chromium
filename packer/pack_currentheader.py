from pack_currenttypes import *;

print """
#ifndef CR_CURRENT_H
#define CR_CURRENT_H
"""

for k in current_fns.keys():
	name = k.lower()
	print "typedef struct {"
	print "\tunsigned char *ptr;"
	for type in current_fns[k]['types']:
		for size in current_fns[k]['sizes']:
			print "\tunsigned char *%s%d;" % (type, size)
	print "} GL%s_p;\n" % name

print "typedef struct {"
for k in current_fns.keys():
	name = k.lower();
	print "\tGL%s_p %s;" % (name,name)

print """
	unsigned char *vtx_op;
	unsigned char *vtx_data;
	unsigned char *begin_op;
	unsigned char *begin_data;
	unsigned int vtx_count;
	unsigned int vtx_max;
	unsigned int vtx_count_begin;

} CRCurrentStatePointers;

#endif /* CR_CURRENT_H */
"""
