/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "packer.h"

void PACK_APIENTRY
crPackDeleteFencesNV(GLsizei n, const GLuint * fences)
{
	unsigned char *data_ptr;
	int packet_length = sizeof(int) + sizeof(n) + n * sizeof(*fences);

	data_ptr = (unsigned char *) crPackAlloc(packet_length);
	WRITE_DATA(0, int, packet_length);
	WRITE_DATA(4, GLenum, CR_DELETEFENCESNV_EXTEND_OPCODE);
	WRITE_DATA(sizeof(int) + 4, GLsizei, n);
	crMemcpy(data_ptr + sizeof(int) + 8, fences, n * sizeof(*fences));
	crHugePacket(CR_EXTEND_OPCODE, data_ptr);
}

void PACK_APIENTRY crPackDeleteFencesNVSWAP( GLsizei n, const GLuint *fences )
{
	unsigned char *data_ptr;
	int i;
	int packet_length = sizeof(int) + sizeof(n) + n * sizeof(*fences);

	data_ptr = (unsigned char *) crPackAlloc( packet_length );
	WRITE_DATA(0, int, packet_length);
	WRITE_DATA(4, GLenum, CR_DELETEFENCESNV_EXTEND_OPCODE);
	WRITE_DATA(sizeof(int) + 4, GLsizei, n);
	for (i = 0 ; i < n ; i++)
	{
		WRITE_DATA((i+1)*sizeof( int ) + 8, GLint, SWAP32(fences[i]));
	}
	crHugePacket(CR_EXTEND_OPCODE, data_ptr);
	crPackFree(data_ptr);
}

