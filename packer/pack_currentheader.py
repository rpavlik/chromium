current_fns = {
	'Color': {
		'types': ['b','ub','s','us','i','ui','f','d'],
		'sizes': [3,4],
		'default': [0,0,0,1],
		'members': ['r', 'g', 'b', 'a']
	},
	'Normal': {
		'types': ['b','s','i','f','d'],
		'sizes': [3],
		'default': [0,0,0],
		'members': ['x', 'y', 'z']
	},
	'TexCoord': {
		'types': ['s','i','f','d'],
		'sizes': [1,2,3,4],
		'default': [0,0,0,1],
		'members': ['s', 't', 'p', 'q']
	},
	'EdgeFlag': {
		'types': ['l'],
		'sizes': [1],
		'default': [1],
		'members': []
	},
	'Index': {
		'types': ['ub','s','i','f','d'],
		'sizes': [1],
		'default': [0],
		'members': []
	}
}

current_vtx = {
	'Vertex': {
		'types': ['s','i','f','d'],
		'sizes': [2,3,4],
		'default': [0,0,0,1],
		'members': ['x', 'y', 'z', 'w']
	}
}

gltypes = {
	'l': {
		'type': 'GLboolean',
		'size': 1
	},
	'b': {
		'type': 'GLbyte',
		'size': 1
	},
	'ub': {
		'type': 'GLubyte',
		'size': 1
	},
	's': {
		'type': 'GLshort',
		'size': 2
	},
	'us': {
		'type': 'GLushort',
		'size': 2
	},
	'i': {
		'type': 'GLint',
		'size': 4
	},
	'ui': {
		'type': 'GLuint',
		'size': 4
	},
	'f': {
		'type': 'GLfloat',
		'size': 4
	},
	'd': {
		'type': 'GLdouble',
		'size': 8
	}
}

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
