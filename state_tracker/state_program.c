/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "state.h"
#include "cr_mem.h"
#include "cr_string.h"


/*
 * General notes:
 *
 * Vertex programs can change vertices so bounding boxes may not be
 * practical for tilesort.  Tilesort may have to broadcast geometry
 * when vertex programs are in effect.  We could semi-parse vertex
 * programs to determine if they write to the o[HPOS] register.
 */


/*
 * Fragment programs have named symbols which are defined/declared
 * within the fragment program that can also be set with the
 * glProgramNamedParameter4*NV() functions.
 * We keep a linked list of these CRProgramSymbol structures to implement
 * a symbol table.  A simple linked list is sufficient since a fragment
 * program typically has just a few symbols.
 */
typedef struct CRProgramSymbol {
	const char *name;
	GLfloat value[4];
	CRbitvalue dirty[CR_MAX_BITARRAY];
	struct CRProgramSymbol *next;
} CRProgramSymbol;



/*
 * Lookup the named program and return a pointer to it.
 * If the program doesn't exist, create it and reserve its Id and put
 * it into the hash table.
 */
static CRProgram *
GetProgram(CRProgramState *p, GLenum target, GLuint id)
{
	CRProgram *prog;

	prog = crHashtableSearch(p->programHash, id);
	if (!prog) {
		prog = (CRProgram *) crCalloc(sizeof(CRProgram));
		if (!prog)
			return NULL;
		prog->target = target;
		prog->id = id;
		prog->resident = GL_TRUE;
		prog->symbolTable = NULL;
		/* reserve the id */
		crIdPoolAllocId(p->programPool, id);
		/* insert into hash table */
		crHashtableAdd(p->programHash, id, (void *) prog);
	}
	return prog;
}


/*
 * Delete a CRProgram object and all attached data.
 */
static void
DeleteProgram(CRProgram *prog)
{
	CRProgramSymbol *symbol, *next;

	if (prog->string)
		crFree((void *) prog->string);

	for (symbol = prog->symbolTable; symbol; symbol = next) {
		next = symbol->next;
		crFree((void *) symbol->name);
		crFree(symbol);
	}
	crFree(prog);
}


/*
 * Set the named symbol to the value (x, y, z, w).
 * NOTE: Symbols should only really be added during parsing of the program.
 * However, the state tracker does not parse the programs (yet).  So, when
 * someone calls glProgramNamedParameter4fNV() we always enter the symbol
 * since we don't know if it's really valid or not.
 */
static void
SetProgramSymbol(CRProgram *prog, const char *name, GLsizei len,
								 GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	CRProgramSymbol *symbol;

	for (symbol = prog->symbolTable; symbol; symbol = symbol->next) {
		/* NOTE: <name> may not be null-terminated! */
		if (crStrncmp(symbol->name, name, len) == 0
				&& crStrlen(symbol->name) == len) {
			/* found it */
			symbol->value[0] = x;
			symbol->value[1] = y;
			symbol->value[2] = z;
			symbol->value[3] = w;
			FILLDIRTY(symbol->dirty);
			return;
		}
	}
	/* add new symbol table entry */
	symbol = (CRProgramSymbol *) crAlloc(sizeof(CRProgramSymbol));
	if (symbol) {
	   symbol->name = crStrndup(name, len);
	   symbol->value[0] = x;
	   symbol->value[1] = y;
	   symbol->value[2] = z;
	   symbol->value[3] = w;
	   symbol->next = prog->symbolTable;
	   prog->symbolTable = symbol;
	   FILLDIRTY(symbol->dirty);
	}
}


/*
 * Return a pointer to the values for the given symbol.  Return NULL if
 * the name doesn't exist in the symbol table.
 */
static const GLfloat *
GetProgramSymbol(const CRProgram *prog, const char *name, GLsizei len)
{
	CRProgramSymbol *symbol = prog->symbolTable;
	for (symbol = prog->symbolTable; symbol; symbol = symbol->next) {
		/* NOTE: <name> may not be null-terminated! */
		if (crStrncmp(symbol->name, name, len) == 0
				&& crStrlen(symbol->name) == len) {
			return symbol->value;
		}
	}
	return NULL;
}


void STATE_APIENTRY crStateBindProgramNV(GLenum target, GLuint id)
{
	CRContext *g = GetCurrentContext();
	CRProgramState *p = &(g->program);
	CRProgram *prog;

	if (g->current.inBeginEnd) {
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
								 "glBindProgramNV called in Begin/End");
		return;
	}

	prog = GetProgram(p, target, id );
	if (!prog) {
		crStateError(__LINE__, __FILE__, GL_OUT_OF_MEMORY, "glBindProgramNV");
		return;
	}
	else if (prog->target != target) {
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
								 "glBindProgramNV bad target");
		return;
	}

	if (target == GL_VERTEX_PROGRAM_NV) {
		p->currentVertexProgram = prog;
		/* XXX set dirty bits! */
	}
	else if (target == GL_FRAGMENT_PROGRAM_NV) {
		p->currentFragmentProgram = prog;
		/* XXX set dirty bits! */
	}
	else {
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
								 "glBindProgramNV(bad target)");
		return;
	}
}


void STATE_APIENTRY crStateDeleteProgramsNV(GLsizei n, const GLuint *ids)
{
	CRContext *g = GetCurrentContext();
	CRProgramState *p = &(g->program);
	GLint i;

	if (g->current.inBeginEnd) {
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
								 "glDeleteProgramsNV called in Begin/End");
		return;
	}

	if (n < 0)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "glDeleteProgramsNV(n)");
		return;
	}

	for (i = 0; i < n; i++) {
		CRProgram *prog = (CRProgram *) crHashtableSearch(p->programHash, ids[i]);
		if (prog) {
			DeleteProgram(prog);
		}
		crHashtableDelete(p->programHash, ids[i], GL_FALSE);
		crIdPoolFreeBlock(p->programPool, ids[i], 1);
	}
}


void STATE_APIENTRY crStateExecuteProgramNV(GLenum target, GLuint id, const GLfloat *params)
{
	/* Hmmm, this is really hard to do if we don't actually execute
	 * the program in a software simulation.
	 */
}


void STATE_APIENTRY crStateGenProgramsNV(GLsizei n, GLuint *ids)
{
	CRContext *g = GetCurrentContext();
	CRProgramState *p = &(g->program);
	GLint start, i;

	if (g->current.inBeginEnd) {
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
								 "glGenProgramsNV called in Begin/End");
		return;
	}

	if (n < 0) {
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "glGenProgramsNV(n)");
		return;
	}

	start = crIdPoolAllocBlock(p->programPool, n);
	for (i = 0; i < n; i++)
		ids[i] = (GLuint) (start + i);
}


GLboolean STATE_APIENTRY crStateAreProgramsResidentNV(GLsizei n, const GLuint *ids, GLboolean *residences)
{
	CRContext *g = GetCurrentContext();
	CRProgramState *p = &(g->program);
	int i;
	GLboolean retVal = GL_TRUE;

	if (n < 0) {
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
								 "glAreProgramsResidentNV(n)");
		return GL_FALSE;
	}

	for (i = 0; i < n; i++) {
		CRProgram *prog;

		if (ids[i] == 0) {
			crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
									 "glAreProgramsResidentNV(id)");
			return GL_FALSE;
		}

		prog = (CRProgram *) crHashtableSearch(p->programHash, ids[i]);
		if (!prog) {
			crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
									 "glAreProgramsResidentNV(id)");
			return GL_FALSE;
		}

		if (!prog->resident) {
			 retVal = GL_FALSE;
			 break;
		}
	}

	if (retVal == GL_FALSE) {
		for (i = 0; i < n; i++) {
			CRProgram *prog = (CRProgram *)
				crHashtableSearch(p->programHash, ids[i]);
			residences[i] = prog->resident;
		}
	}

	return retVal;
}


void STATE_APIENTRY crStateRequestResidentProgramsNV(GLsizei n, const GLuint *ids)
{
	(void) n;
	(void) ids;
}



GLboolean crStateIsProgramNV(GLuint id)
{
	CRContext *g = GetCurrentContext();
	CRProgramState *p = &(g->program);
	CRProgram *prog;

	if (g->current.inBeginEnd) {
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
								 "glIsProgramNV called in Begin/End");
		return GL_FALSE;
	}

	if (id == 0) {
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
								 "glIsProgramNV(id==0)");
		return GL_FALSE;
	}

	prog = (CRProgram *) crHashtableSearch(p->programHash, id);
	if (prog)
      return GL_TRUE;
   else
      return GL_FALSE;
}


void STATE_APIENTRY crStateLoadProgramNV(GLenum target, GLuint id, GLsizei len,
																				 const GLubyte *program)
{
	CRContext *g = GetCurrentContext();
	CRProgramState *p = &(g->program);
	CRProgram *prog;
	GLubyte *progCopy;

	if (g->current.inBeginEnd) {
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
								 "glLoadProgramNV called in Begin/End");
		return;
	}

	if (id == 0) {
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
								 "glLoadProgramNV(id==0)");
		return;
	}

	prog = GetProgram(p, target, id);

	if (!prog) {
		crStateError(__LINE__, __FILE__, GL_OUT_OF_MEMORY, "glLoadProgramNV");
		return;
	}
	else if (prog && prog->target != target) {
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
								 "glLoadProgramNV(target)");
		return;
	}

	progCopy = crAlloc(len);
	if (!progCopy) {
			crStateError(__LINE__, __FILE__, GL_OUT_OF_MEMORY, "glLoadProgramNV");
			return;
	}
	crMemcpy(progCopy, program, len);
	if (prog->string)
		crFree((void *) prog->string);

	prog->string = progCopy;
	prog->length = len;
	/* XXX set dirty bits! */
}


void STATE_APIENTRY crStateGetProgramivNV(GLuint id, GLenum pname, GLint *params)
{
	CRContext *g = GetCurrentContext();
	CRProgramState *p = &(g->program);
	CRProgram *prog;

	if (g->current.inBeginEnd) {
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
								 "glGetProgramivNV called in Begin/End");
		return;
	}

	if (id == 0) {
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
								 "glGetProgramivNV(bad id)");
		return;
	}
		
	prog = (CRProgram *) crHashtableSearch(p->programHash, id);
	if (!prog) {
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
								 "glGetProgramivNV(bad id)");
		return;
	}

	switch (pname) {
	case GL_PROGRAM_TARGET_NV:
		*params = prog->target;
		return;
	case GL_PROGRAM_LENGTH_NV:
		*params = prog->length;
		return;
	case GL_PROGRAM_RESIDENT_NV:
		*params = prog->resident;
		return;
	default:
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
								 "glGetProgramivNV(pname)");
		return;
	}
}


void STATE_APIENTRY crStateGetProgramStringNV(GLuint id, GLenum pname, GLubyte *program)
{
	CRContext *g = GetCurrentContext();
	CRProgramState *p = &(g->program);
	CRProgram *prog;

	if (g->current.inBeginEnd) {
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
								 "glGetProgramStringNV called in Begin/End");
		return;
	}

	if (id == 0) {
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
								 "glGetProgramStringNV(bad id)");
		return;
	}
		
	prog = (CRProgram *) crHashtableSearch(p->programHash, id);
	if (!prog) {
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
								 "glGetProgramStringNV(bad id)");
		return;
	}

	crMemcpy(program, prog->string, prog->length);
}


void STATE_APIENTRY crStateProgramParameter4dNV(GLenum target, GLuint index,
                                GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
	crStateProgramParameter4fNV(target, index, x, y, z, w);
}


void STATE_APIENTRY crStateProgramParameter4dvNV(GLenum target, GLuint index,
                                 const GLdouble *params)
{
	crStateProgramParameter4fNV(target, index,
															params[0], params[1], params[2], params[3]);
}


void STATE_APIENTRY crStateProgramParameter4fNV(GLenum target, GLuint index,
																 GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	CRContext *g = GetCurrentContext();
	CRProgramState *p = &(g->program);

	if (g->current.inBeginEnd) {
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
								 "glProgramParameterNV called in Begin/End");
		return;
	}

	if (target == GL_VERTEX_PROGRAM_NV) {
		if (index < MAX_VERTEX_PROGRAM_PARAMETERS) {
			p->VertexProgramParameters[index][0] = x;
			p->VertexProgramParameters[index][1] = y;
			p->VertexProgramParameters[index][2] = z;
			p->VertexProgramParameters[index][3] = w;
			/* XXX set dirty bits! */
		}
		else {
			crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
									 "glProgramParameterNV(index)");
			return;
		}
	}
	else {
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
								 "glProgramParameterNV(target)");
		return;
	}
}


void STATE_APIENTRY crStateProgramParameter4fvNV(GLenum target, GLuint index,
                                 const GLfloat *params)
{
	crStateProgramParameter4fNV(target, index,
															params[0], params[1], params[2], params[3]);
}


void STATE_APIENTRY crStateProgramParameters4dvNV(GLenum target, GLuint index,
                                  GLuint num, const GLdouble *params)
{
	CRContext *g = GetCurrentContext();
	CRProgramState *p = &(g->program);

	if (g->current.inBeginEnd) {
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
								 "glProgramParameters4dvNV called in Begin/End");
		return;
	}

	if (target == GL_VERTEX_PROGRAM_NV) {
		if (index + num < MAX_VERTEX_PROGRAM_PARAMETERS) {
			GLuint i;
			for (i = 0; i < num; i++) {
				p->VertexProgramParameters[index+i][0] = params[i*4+0];
				p->VertexProgramParameters[index+i][1] = params[i*4+1];
				p->VertexProgramParameters[index+i][2] = params[i*4+2];
				p->VertexProgramParameters[index+i][3] = params[i*4+3];
			}
			/* XXX set dirty bits! */
		}
		else {
			crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
									 "glProgramParameters4dvNV(index+num)");
			return;
		}
	}
	else {
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
								 "glProgramParameterNV(target)");
		return;
	}
}


void STATE_APIENTRY crStateProgramParameters4fvNV(GLenum target, GLuint index,
                                  GLuint num, const GLfloat *params)
{
	CRContext *g = GetCurrentContext();
	CRProgramState *p = &(g->program);

	if (g->current.inBeginEnd) {
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
								 "glProgramParameters4dvNV called in Begin/End");
		return;
	}

	if (target == GL_VERTEX_PROGRAM_NV) {
		if (index + num < MAX_VERTEX_PROGRAM_PARAMETERS) {
			GLuint i;
			for (i = 0; i < num; i++) {
				p->VertexProgramParameters[index+i][0] = params[i*4+0];
				p->VertexProgramParameters[index+i][1] = params[i*4+1];
				p->VertexProgramParameters[index+i][2] = params[i*4+2];
				p->VertexProgramParameters[index+i][3] = params[i*4+3];
			}
			/* XXX set dirty bits! */
		}
		else {
			crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
									 "glProgramParameters4dvNV(index+num)");
			return;
		}
	}
	else {
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
								 "glProgramParameterNV(target)");
		return;
	}
}


void STATE_APIENTRY crStateGetProgramParameterfvNV(GLenum target, GLuint index,
																		GLenum pname, GLfloat *params)
{
	CRContext *g = GetCurrentContext();
	CRProgramState *p = &(g->program);

	if (g->current.inBeginEnd) {
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
								 "glGetProgramParameterfvNV called in Begin/End");
		return;
	}

	if (target == GL_VERTEX_PROGRAM_NV) {
		if (pname == GL_PROGRAM_PARAMETER_NV) {
			if (index < MAX_VERTEX_PROGRAM_PARAMETERS) {
				params[0] = p->VertexProgramParameters[index][0];
				params[1] = p->VertexProgramParameters[index][1];
				params[2] = p->VertexProgramParameters[index][2];
				params[3] = p->VertexProgramParameters[index][3];
			}
			else {
				crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
										 "glGetProgramParameterfvNV(index)");
				return;
			}
		}
		else {
			crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
									 "glGetProgramParameterfvNV(pname)");
			return;
		}
	}
}


void STATE_APIENTRY crStateGetProgramParameterdvNV(GLenum target, GLuint index,
                                   GLenum pname, GLdouble *params)
{
	CRContext *g = GetCurrentContext();
	CRProgramState *p = &(g->program);

	if (g->current.inBeginEnd) {
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
								 "glGetProgramParameterdvNV called in Begin/End");
		return;
	}

	if (target == GL_VERTEX_PROGRAM_NV) {
		if (pname == GL_PROGRAM_PARAMETER_NV) {
			if (index < MAX_VERTEX_PROGRAM_PARAMETERS) {
				params[0] = p->VertexProgramParameters[index][0];
				params[1] = p->VertexProgramParameters[index][1];
				params[2] = p->VertexProgramParameters[index][2];
				params[3] = p->VertexProgramParameters[index][3];
			}
			else {
				crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
										 "glGetProgramParameterdvNV(index)");
				return;
			}
		}
		else {
			crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
									 "glGetProgramParameterdvNV(pname)");
			return;
		}
	}
}


void STATE_APIENTRY crStateTrackMatrixNV(GLenum target, GLuint address,
                         GLenum matrix, GLenum transform)
{
	CRContext *g = GetCurrentContext();
	CRProgramState *p = &(g->program);

	if (g->current.inBeginEnd) {
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
								 "glGetTrackMatrixivNV called in Begin/End");
		return;
	}
	
	if (target == GL_VERTEX_PROGRAM_NV) {
		if (address & 0x3) {
      /* addr must be multiple of four */
      crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
									 "glTrackMatrixNV(address)");
      return;
		}

		switch (matrix) {
		case GL_NONE:
		case GL_MODELVIEW:
		case GL_PROJECTION:
		case GL_TEXTURE:
		case GL_COLOR:
		case GL_MODELVIEW_PROJECTION_NV:
		case GL_MATRIX0_NV:
		case GL_MATRIX1_NV:
		case GL_MATRIX2_NV:
		case GL_MATRIX3_NV:
		case GL_MATRIX4_NV:
		case GL_MATRIX5_NV:
		case GL_MATRIX6_NV:
		case GL_MATRIX7_NV:
			/* OK, fallthrough */
			break;
		default:
			crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
									 "glTrackMatrixNV(matrix)");
			return;
		}

		switch (transform) {
		case GL_IDENTITY_NV:
		case GL_INVERSE_NV:
		case GL_TRANSPOSE_NV:
		case GL_INVERSE_TRANSPOSE_NV:
			/* OK, fallthrough */
			break;
		default:
			crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
									 "glTrackMatrixNV(transform)");
			return;
		}

		p->TrackMatrix[address / 4] = matrix;
		p->TrackMatrixTransform[address / 4] = transform;
		/* XXX set dirty bits! */
	}
	else {
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
								 "glTrackMatrixNV(target)");
	}
}


void STATE_APIENTRY crStateGetTrackMatrixivNV(GLenum target, GLuint address,
															 GLenum pname, GLint *params)
{
	CRContext *g = GetCurrentContext();
	CRProgramState *p = &(g->program);

	if (g->current.inBeginEnd) {
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
								 "glGetTrackMatrixivNV called in Begin/End");
		return;
	}

	if (target == GL_VERTEX_PROGRAM_NV) {
		if ((address & 0x3) || address >= MAX_VERTEX_PROGRAM_PARAMETERS) {
			crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
									 "glGetTrackMatrixivNV(address)");
			return;
		}
		if (pname == GL_TRACK_MATRIX_NV) {
			params[0] = (GLint) p->TrackMatrix[address / 4];
		}
		else if (pname == GL_TRACK_MATRIX_TRANSFORM_NV) {
			params[0] = (GLint) p->TrackMatrixTransform[address / 4];
		}
		else {
			crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
									 "glGetTrackMatrixivNV(pname)");
			return;
		}
	}
	else {
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
								 "glGetTrackMatrixivNV(target)");
		return;
	}
}


void STATE_APIENTRY crStateGetVertexAttribdvNV(GLuint index, GLenum pname, GLdouble *params)
{
	 GLfloat floatParams[4];
	 crStateGetVertexAttribfvNV(index, pname, floatParams);
	 params[0] = floatParams[0];
	 if (pname == GL_CURRENT_ATTRIB_NV) {
			params[1] = floatParams[1];
			params[2] = floatParams[2];
			params[3] = floatParams[3];
	 }
}


void STATE_APIENTRY crStateGetVertexAttribfvNV(GLuint index, GLenum pname, GLfloat *params)
{
	CRContext *g = GetCurrentContext();

	if (g->current.inBeginEnd) {
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
								 "glGetVertexAttribfvNV called in Begin/End");
		return;
	}

	if (index >= CR_MAX_VERTEX_ATTRIBS) {
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
								 "glGetVertexAttribdvNV(index)");
		return;
	}

	switch (pname) {
	case GL_ATTRIB_ARRAY_SIZE_NV:
		params[0] = (GLfloat) g->client.a[index].size;
		break;
	case GL_ATTRIB_ARRAY_STRIDE_NV:
		params[0] = (GLfloat) g->client.a[index].stride;
		break;
	case GL_ATTRIB_ARRAY_TYPE_NV:
		params[0] = (GLfloat) g->client.a[index].type;
		break;
	case GL_CURRENT_ATTRIB_NV:
		params[0] = g->current.vertexAttrib[index].x;
		params[1] = g->current.vertexAttrib[index].y;
		params[2] = g->current.vertexAttrib[index].z;
		params[3] = g->current.vertexAttrib[index].w;
		break;
	default:
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glGetVertexAttribdvNV");
		return;
	}
}


void STATE_APIENTRY crStateGetVertexAttribivNV(GLuint index, GLenum pname, GLint *params)
{
	 GLfloat floatParams[4];
	 crStateGetVertexAttribfvNV(index, pname, floatParams);
	 params[0] = (GLint) floatParams[0];
	 if (pname == GL_CURRENT_ATTRIB_NV) {
			params[1] = (GLint) floatParams[1];
			params[2] = (GLint) floatParams[2];
			params[3] = (GLint) floatParams[3];
	 }
}


void STATE_APIENTRY crStateGetVertexAttribPointervNV(GLuint index, GLenum pname, GLvoid **pointer)
{
	CRContext *g = GetCurrentContext();

	if (g->current.inBeginEnd) {
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
								 "glGetVertexAttribPointervNV called in Begin/End");
		return;
	}

	if (index >= CR_MAX_VERTEX_ATTRIBS) {
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
								 "glGetVertexAttribPointervNV(index)");
		return;
	}

	if (pname != GL_ATTRIB_ARRAY_POINTER_NV) {
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
								 "glGetVertexAttribPointervNV(pname)");
		return;
	}

	*pointer = g->client.a[index].p;
}


/**********************************************************************/

/*
 * Added by GL_NV_fragment_program
 */

void STATE_APIENTRY crStateProgramNamedParameter4fNV(GLuint id, GLsizei len, const GLubyte *name, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	CRContext *g = GetCurrentContext();
	CRProgramState *p = &(g->program);
	CRProgram *prog;

	if (g->current.inBeginEnd) {
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
								 "glProgramNamedParameterfNV called in Begin/End");
		return;
	}

	prog = (CRProgram *) crHashtableSearch(p->programHash, id);
	if (!prog) {
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
								 "glProgramNamedParameterNV(bad id)");
		return;
	}

	if (prog->target != GL_FRAGMENT_PROGRAM_NV) {
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
								 "glProgramNamedParameterNV(target)");
		return;
	}

	SetProgramSymbol(prog, name, len, x, y, z, w);
}


void STATE_APIENTRY crStateProgramNamedParameter4dNV(GLuint id, GLsizei len, const GLubyte *name, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
	crStateProgramNamedParameter4fNV(id, len, name, x, y, z, w);
}


void STATE_APIENTRY crStateProgramNamedParameter4fvNV(GLuint id, GLsizei len, const GLubyte *name, const GLfloat v[])
{
	crStateProgramNamedParameter4fNV(id, len, name, v[0], v[1], v[2], v[3]);
}


void STATE_APIENTRY crStateProgramNamedParameter4dvNV(GLuint id, GLsizei len, const GLubyte *name, const GLdouble v[])
{
	crStateProgramNamedParameter4fNV(id, len, name, v[0], v[1], v[2], v[3]);
}


void STATE_APIENTRY crStateGetProgramNamedParameterfvNV(GLuint id, GLsizei len, const GLubyte *name, GLfloat *params)
{
	CRContext *g = GetCurrentContext();
	CRProgramState *p = &(g->program);
	const CRProgram *prog;
	const GLfloat *value;

	if (g->current.inBeginEnd) {
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
								 "glGetProgramNamedParameterfNV called in Begin/End");
		return;
	}

	prog = (const CRProgram *) crHashtableSearch(p->programHash, id);
	if (!prog) {
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
								 "glGetProgramNamedParameterNV(bad id)");
		return;
	}

	if (prog->target != GL_FRAGMENT_PROGRAM_NV) {
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
								 "glGetProgramNamedParameterNV(target)");
		return;
	}

	value = GetProgramSymbol(prog, name, len);
	if (!value) {
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
								 "glGetProgramNamedParameterNV(name)");
		return;
	}

	params[0] = value[0];
	params[1] = value[1];
	params[2] = value[2];
	params[3] = value[3];
}


void STATE_APIENTRY crStateGetProgramNamedParameterdvNV(GLuint id, GLsizei len, const GLubyte *name, GLdouble *params)
{
	GLfloat floatParams[4];
	crStateGetProgramNamedParameterfvNV(id, len, name, floatParams);
	params[0] = floatParams[0];
	params[1] = floatParams[1];
	params[2] = floatParams[2];
	params[3] = floatParams[3];
}


void STATE_APIENTRY crStateProgramLocalParameter4dARB(GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
	crStateProgramLocalParameter4fARB(target, index, x, y, z, w);
}


void STATE_APIENTRY crStateProgramLocalParameter4dvARB(GLenum target, GLuint index, const GLdouble *params)
{
	crStateProgramLocalParameter4fARB(target, index, params[0], params[1],
																		params[2], params[3]);
}


void STATE_APIENTRY crStateProgramLocalParameter4fARB(GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	CRContext *g = GetCurrentContext();
	CRProgramState *p = &(g->program);
	CRProgram *prog;

	if (g->current.inBeginEnd) {
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
								 "glProgramLocalParameterNV called in Begin/End");
		return;
	}

	prog = p->currentFragmentProgram;
	if (!prog) {
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
								 "glProgramLocalParameterNV(no program)");
		return;
	}

	if (index >= MAX_FRAGMENT_PROGRAM_PARAMETERS) {
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
								 "glProgramLocalParameterNV(index)");
		return;
	}

	prog->fragmentLocalParameters[index][0] = x;
	prog->fragmentLocalParameters[index][1] = y;
	prog->fragmentLocalParameters[index][2] = z;
	prog->fragmentLocalParameters[index][3] = w;
}


void STATE_APIENTRY crStateProgramLocalParameter4fvARB(GLenum target, GLuint index, const GLfloat *params)
{
	crStateProgramLocalParameter4fARB(target, index, params[0], params[1],
																		params[2], params[3]);
}


void STATE_APIENTRY crStateGetProgramLocalParameterfvARB(GLenum target, GLuint index, GLfloat *params)
{
	CRContext *g = GetCurrentContext();
	CRProgramState *p = &(g->program);
	const CRProgram *prog;

	if (g->current.inBeginEnd) {
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
								 "glGetProgramLocalParameterNV called in Begin/End");
		return;
	}

	prog = p->currentFragmentProgram;
	if (!prog) {
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
								 "glGetProgramLocalParameterNV(no program)");
		return;
	}

	if (index >= MAX_FRAGMENT_PROGRAM_PARAMETERS) {
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
								 "glGetProgramLocalParameterNV(index)");
		return;
	}

	params[0] = prog->fragmentLocalParameters[index][0];
	params[1] = prog->fragmentLocalParameters[index][1];
	params[2] = prog->fragmentLocalParameters[index][2];
	params[3] = prog->fragmentLocalParameters[index][3];
}


void STATE_APIENTRY crStateGetProgramLocalParameterdvARB(GLenum target, GLuint index, GLdouble *params)
{
	GLfloat floatParams[4];
	crStateGetProgramLocalParameterfvARB(target, index, floatParams);
	params[0] = floatParams[0];
	params[1] = floatParams[1];
	params[2] = floatParams[2];
	params[3] = floatParams[3];
}



/**********************************************************************/


void crStateProgramInit(const CRLimitsState *limits, CRProgramState *p)
{
	GLuint i;

	p->currentVertexProgram = NULL;
	p->currentFragmentProgram = NULL;
	p->errorPos = -1;
	p->errorString = NULL;
	p->programHash = crAllocHashtable();

	for (i = 0; i < MAX_VERTEX_PROGRAM_PARAMETERS / 4; i++) {
		p->TrackMatrix[i] = GL_NONE;
		p->TrackMatrixTransform[i] = GL_IDENTITY_NV;
	}
	for (i = 0; i < MAX_VERTEX_PROGRAM_PARAMETERS; i++) {
		p->VertexProgramParameters[i][0] = 0.0;
		p->VertexProgramParameters[i][1] = 0.0;
		p->VertexProgramParameters[i][2] = 0.0;
		p->VertexProgramParameters[i][3] = 0.0;
	}
}


void crStateProgramFree(CRProgramState *p)
{
	/* walk hash table, freeing program objects */
	CR_HASHTABLE_WALK(p->programHash, entry)

		CRProgram *prog = (CRProgram *) entry->data;
		if (prog->string)
			crFree((void *) prog->string);
		crFree(prog);
		entry->data = NULL;

	CR_HASHTABLE_WALK_END(t->idHash);

	crFreeIdPool(p->programPool);
	crFreeHashtable(p->programHash);
}


