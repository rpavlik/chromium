# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

import sys;

sys.path.append( '../../packer' )
from pack_currenttypes import *;

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
	print '\t\tcrError ( "Unknown opcode in VPINCH_CONVERT_%s" ); \\' % ucname
	print '\t}\\'

	i = 0
	for member in current_fns[k]['members']:
		print '\t(dst).%s = vdata[%d];\\' % (member,i)
		i += 1

	print '}\n'
