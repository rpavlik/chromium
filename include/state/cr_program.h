/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef CR_STATE_PROGRAM_H
#define CR_STATE_PROGRAM_H

#include "cr_hash.h"
#include "state/cr_statetypes.h"
#include "state/cr_limits.h"


#define MAX_VERTEX_PROGRAM_PARAMETERS  96
#define MAX_FRAGMENT_PROGRAM_PARAMETERS 16


typedef struct {
	int foo;
} CRProgramBits;


struct CRProgramSymbol;


/*
 * A vertex or fragment program.
 */
typedef struct {
	GLenum target;
	GLuint id;
	const GLubyte *string;
	GLsizei length;
	GLboolean resident;
	struct CRProgramSymbol *symbolTable;
	GLfloat fragmentLocalParameters[MAX_FRAGMENT_PROGRAM_PARAMETERS][4];
	CRbitvalue dirtyParams[CR_MAX_BITARRAY];
} CRProgram;



typedef struct {
	CRProgram *currentVertexProgram;
	CRProgram *currentFragmentProgram;
	GLint errorPos;
	const GLubyte *errorString;

	/* tracking matrices for vertex programs */
	GLenum TrackMatrix[MAX_VERTEX_PROGRAM_PARAMETERS / 4];
	GLenum TrackMatrixTransform[MAX_VERTEX_PROGRAM_PARAMETERS / 4];

	GLfloat VertexProgramParameters[MAX_VERTEX_PROGRAM_PARAMETERS][4];

	CRHashTable *programHash;  /* XXX belongs in shared state, actually */
} CRProgramState;



extern void crStateProgramInit(const CRLimitsState *limits, CRProgramState *t);
extern void crStateProgramFree(CRProgramState *t);


#if 1  /* temporary */


extern GLboolean crStateAreProgramsResidentNV (GLsizei, const GLuint *, GLboolean *);
extern void crStateBindProgramNV (GLenum, GLuint);
extern void crStateDeleteProgramsNV (GLsizei, const GLuint *);
extern void crStateExecuteProgramNV (GLenum, GLuint, const GLfloat *);
extern void crStateGenProgramsNV (GLsizei, GLuint *);
extern void crStateGetProgramParameterdvNV (GLenum, GLuint, GLenum, GLdouble *);
extern void crStateGetProgramParameterfvNV (GLenum, GLuint, GLenum, GLfloat *);
extern void crStateGetProgramivNV (GLuint, GLenum, GLint *);
extern void crStateGetProgramStringNV (GLuint, GLenum, GLubyte *);
extern void crStateGetTrackMatrixivNV (GLenum, GLuint, GLenum, GLint *);
extern void crStateGetVertexAttribdvNV (GLuint, GLenum, GLdouble *);
extern void crStateGetVertexAttribfvNV (GLuint, GLenum, GLfloat *);
extern void crStateGetVertexAttribivNV (GLuint, GLenum, GLint *);
extern void crStateGetVertexAttribPointervNV (GLuint, GLenum, GLvoid* *);
extern GLboolean crStateIsProgramNV (GLuint);
extern void crStateLoadProgramNV (GLenum, GLuint, GLsizei, const GLubyte *);
extern void crStateProgramParameter4dNV (GLenum, GLuint, GLdouble, GLdouble, GLdouble, GLdouble);
extern void crStateProgramParameter4dvNV (GLenum, GLuint, const GLdouble *);
extern void crStateProgramParameter4fNV (GLenum, GLuint, GLfloat, GLfloat, GLfloat, GLfloat);
extern void crStateProgramParameter4fvNV (GLenum, GLuint, const GLfloat *);
extern void crStateProgramParameters4dvNV (GLenum, GLuint, GLuint, const GLdouble *);
extern void crStateProgramParameters4fvNV (GLenum, GLuint, GLuint, const GLfloat *);
extern void crStateRequestResidentProgramsNV (GLsizei, const GLuint *);
extern void crStateTrackMatrixNV (GLenum, GLuint, GLenum, GLenum);
extern void crStateVertexAttribPointerNV (GLuint, GLint, GLenum, GLsizei, const GLvoid *);
extern void crStateVertexAttrib1dNV (GLuint, GLdouble);
extern void crStateVertexAttrib1dvNV (GLuint, const GLdouble *);
extern void crStateVertexAttrib1fNV (GLuint, GLfloat);
extern void crStateVertexAttrib1fvNV (GLuint, const GLfloat *);
extern void crStateVertexAttrib1sNV (GLuint, GLshort);
extern void crStateVertexAttrib1svNV (GLuint, const GLshort *);
extern void crStateVertexAttrib2dNV (GLuint, GLdouble, GLdouble);
extern void crStateVertexAttrib2dvNV (GLuint, const GLdouble *);
extern void crStateVertexAttrib2fNV (GLuint, GLfloat, GLfloat);
extern void crStateVertexAttrib2fvNV (GLuint, const GLfloat *);
extern void crStateVertexAttrib2sNV (GLuint, GLshort, GLshort);
extern void crStateVertexAttrib2svNV (GLuint, const GLshort *);
extern void crStateVertexAttrib3dNV (GLuint, GLdouble, GLdouble, GLdouble);
extern void crStateVertexAttrib3dvNV (GLuint, const GLdouble *);
extern void crStateVertexAttrib3fNV (GLuint, GLfloat, GLfloat, GLfloat);
extern void crStateVertexAttrib3fvNV (GLuint, const GLfloat *);
extern void crStateVertexAttrib3sNV (GLuint, GLshort, GLshort, GLshort);
extern void crStateVertexAttrib3svNV (GLuint, const GLshort *);
extern void crStateVertexAttrib4dNV (GLuint, GLdouble, GLdouble, GLdouble, GLdouble);
extern void crStateVertexAttrib4dvNV (GLuint, const GLdouble *);
extern void crStateVertexAttrib4fNV (GLuint, GLfloat, GLfloat, GLfloat, GLfloat);
extern void crStateVertexAttrib4fvNV (GLuint, const GLfloat *);
extern void crStateVertexAttrib4sNV (GLuint, GLshort, GLshort, GLshort, GLshort);
extern void crStateVertexAttrib4svNV (GLuint, const GLshort *);
extern void crStateVertexAttrib4ubNV (GLuint, GLubyte, GLubyte, GLubyte, GLubyte);
extern void crStateVertexAttrib4ubvNV (GLuint, const GLubyte *);
extern void crStateVertexAttribs1dvNV (GLuint, GLsizei, const GLdouble *);
extern void crStateVertexAttribs1fvNV (GLuint, GLsizei, const GLfloat *);
extern void crStateVertexAttribs1svNV (GLuint, GLsizei, const GLshort *);
extern void crStateVertexAttribs2dvNV (GLuint, GLsizei, const GLdouble *);
extern void crStateVertexAttribs2fvNV (GLuint, GLsizei, const GLfloat *);
extern void crStateVertexAttribs2svNV (GLuint, GLsizei, const GLshort *);
extern void crStateVertexAttribs3dvNV (GLuint, GLsizei, const GLdouble *);
extern void crStateVertexAttribs3fvNV (GLuint, GLsizei, const GLfloat *);
extern void crStateVertexAttribs3svNV (GLuint, GLsizei, const GLshort *);
extern void crStateVertexAttribs4dvNV (GLuint, GLsizei, const GLdouble *);
extern void crStateVertexAttribs4fvNV (GLuint, GLsizei, const GLfloat *);
extern void crStateVertexAttribs4svNV (GLuint, GLsizei, const GLshort *);
extern void crStateVertexAttribs4ubvNV (GLuint, GLsizei, const GLubyte *);

void crStateProgramNamedParameter4fNV(GLuint id, GLsizei len, const GLubyte *name, GLfloat x, GLfloat y, GLfloat z, GLfloat w);

void crStateProgramNamedParameter4dNV(GLuint id, GLsizei len, const GLubyte *name, GLdouble x, GLdouble y, GLdouble z, GLdouble w);

void crStateProgramNamedParameter4fvNV(GLuint id, GLsizei len, const GLubyte *name, const GLfloat v[]);

void crStateProgramNamedParameter4dvNV(GLuint id, GLsizei len, const GLubyte *name, const GLdouble v[]);

void crStateGetProgramNamedParameterfvNV(GLuint id, GLsizei len, const GLubyte *name, GLfloat *params);

void crStateGetProgramNamedParameterdvNV(GLuint id, GLsizei len, const GLubyte *name, GLdouble *params);

void crStateProgramLocalParameter4dARB(GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);

void crStateProgramLocalParameter4dvARB(GLenum target, GLuint index, const GLdouble *params);

void crStateProgramLocalParameter4fARB(GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);

void crStateProgramLocalParameter4fvARB(GLenum target, GLuint index, const GLfloat *params);

void crStateGetProgramLocalParameterdvARB(GLenum target, GLuint index, GLdouble *params);

void crStateGetProgramLocalParameterfvARB(GLenum target, GLuint index, GLfloat *params);

#endif


#endif /* CR_STATE_PROGRAM_H */
