/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef CR_STATE_FEEDBACK_H 
#define CR_STATE_FEEDBACK_H 

#include "state/cr_statetypes.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_NAME_STACK_DEPTH 64

typedef struct {
	GLbitvalue dirty[CR_MAX_BITARRAY];
} CRFeedbackBits;

typedef struct {
	GLbitvalue dirty[CR_MAX_BITARRAY];
} CRSelectionBits;

typedef struct {
	GLenum	type;
	GLuint	mask;
	GLfloat	*buffer;
	GLuint	bufferSize;
	GLuint	count;
} CRFeedbackState;

typedef struct {
   	GLuint *buffer;
   	GLuint bufferSize;
   	GLuint bufferCount;
   	GLuint hits;
   	GLuint nameStackDepth;
   	GLuint nameStack[MAX_NAME_STACK_DEPTH];
   	GLboolean hitFlag;
   	GLfloat hitMinZ, hitMaxZ;
} CRSelectionState;

extern void crStateFeedbackDiff(CRFeedbackBits *bb, GLbitvalue *bitID, 
                                CRFeedbackState *from, CRFeedbackState *to);
extern void crStateFeedbackSwitch(CRFeedbackBits *bb, GLbitvalue *bitID, 
                                  CRFeedbackState *from, CRFeedbackState *to);

extern void crStateFeedbackGetBooleanv( GLenum pname, GLboolean *params );
extern void crStateFeedbackGetDoublev( GLenum pname, GLdouble *params );
extern void crStateFeedbackGetFloatv( GLenum pname, GLfloat *params );
extern void crStateFeedbackGetIntegerv( GLenum pname, GLint *params );

extern void crStateFeedbackBegin( GLenum mode );
extern void crStateFeedbackEnd( void );
extern void crStateFeedbackVertex2d( GLdouble x, GLdouble y );
extern void crStateFeedbackVertex2dv( const GLdouble *v );
extern void crStateFeedbackVertex2f( GLfloat x, GLfloat y );
extern void crStateFeedbackVertex2fv( const GLfloat *v );
extern void crStateFeedbackVertex2i( GLint x, GLint y );
extern void crStateFeedbackVertex2iv( const GLint *v );
extern void crStateFeedbackVertex2s( GLshort x, GLshort y );
extern void crStateFeedbackVertex2sv( const GLshort *v );
extern void crStateFeedbackVertex3d( GLdouble x, GLdouble y, GLdouble z );
extern void crStateFeedbackVertex3dv( const GLdouble *v );
extern void crStateFeedbackVertex3f( GLfloat x, GLfloat y, GLfloat z );
extern void crStateFeedbackVertex3fv( const GLfloat *v );
extern void crStateFeedbackVertex3i( GLint x, GLint y, GLint z );
extern void crStateFeedbackVertex3iv( const GLint *v );
extern void crStateFeedbackVertex3s( GLshort x, GLshort y, GLshort z );
extern void crStateFeedbackVertex3sv( const GLshort *v );
extern void crStateFeedbackVertex4d( GLdouble x, GLdouble y, GLdouble z, GLdouble w );
extern void crStateFeedbackVertex4dv( const GLdouble *v );
extern void crStateFeedbackVertex4f( GLfloat x, GLfloat y, GLfloat z, GLfloat w );
extern void crStateFeedbackVertex4fv( const GLfloat *v );
extern void crStateFeedbackVertex4i( GLint x, GLint y, GLint z, GLint w );
extern void crStateFeedbackVertex4iv( const GLint *v );
extern void crStateFeedbackVertex4s( GLshort x, GLshort y, GLshort z, GLshort w );
extern void crStateFeedbackVertex4sv( const GLshort *v );
extern void crStateFeedbackRectf(GLfloat x0, GLfloat y0, GLfloat x1, GLfloat y1);
extern void crStateFeedbackRecti(GLint x0, GLint y0, GLint x1, GLint y1);
extern void crStateFeedbackRectd(GLdouble x0, GLdouble y0, GLdouble x1, GLdouble y1);
extern void crStateFeedbackRects(GLshort x0, GLshort y0, GLshort x1, GLshort y1);
extern void crStateFeedbackRectfv(const GLfloat *v0, const GLfloat *v1);
extern void crStateFeedbackRectiv(const GLint *v0, const GLint *v1);
extern void crStateFeedbackRectdv(const GLdouble *v0, const GLdouble *v1);
extern void crStateFeedbackRectsv(const GLshort *v0, const GLshort *v1);
extern void crStateFeedbackBitmap( GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap );
extern void crStateFeedbackDrawPixels( GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels );
extern void crStateFeedbackCopyPixels( GLint x, GLint y, GLsizei width, GLsizei height, GLenum type );

extern void crStateSelectBegin( GLenum mode );
extern void crStateSelectEnd( void );
extern void crStateSelectVertex2d( GLdouble x, GLdouble y );
extern void crStateSelectVertex2dv( const GLdouble *v );
extern void crStateSelectVertex2f( GLfloat x, GLfloat y );
extern void crStateSelectVertex2fv( const GLfloat *v );
extern void crStateSelectVertex2i( GLint x, GLint y );
extern void crStateSelectVertex2iv( const GLint *v );
extern void crStateSelectVertex2s( GLshort x, GLshort y );
extern void crStateSelectVertex2sv( const GLshort *v );
extern void crStateSelectVertex3d( GLdouble x, GLdouble y, GLdouble z );
extern void crStateSelectVertex3dv( const GLdouble *v );
extern void crStateSelectVertex3f( GLfloat x, GLfloat y, GLfloat z );
extern void crStateSelectVertex3fv( const GLfloat *v );
extern void crStateSelectVertex3i( GLint x, GLint y, GLint z );
extern void crStateSelectVertex3iv( const GLint *v );
extern void crStateSelectVertex3s( GLshort x, GLshort y, GLshort z );
extern void crStateSelectVertex3sv( const GLshort *v );
extern void crStateSelectVertex4d( GLdouble x, GLdouble y, GLdouble z, GLdouble w );
extern void crStateSelectVertex4dv( const GLdouble *v );
extern void crStateSelectVertex4f( GLfloat x, GLfloat y, GLfloat z, GLfloat w );
extern void crStateSelectVertex4fv( const GLfloat *v );
extern void crStateSelectVertex4i( GLint x, GLint y, GLint z, GLint w );
extern void crStateSelectVertex4iv( const GLint *v );
extern void crStateSelectVertex4s( GLshort x, GLshort y, GLshort z, GLshort w );
extern void crStateSelectVertex4sv( const GLshort *v );
extern void crStateSelectRectf(GLfloat x0, GLfloat y0, GLfloat x1, GLfloat y1);
extern void crStateSelectRecti(GLint x0, GLint y0, GLint x1, GLint y1);
extern void crStateSelectRectd(GLdouble x0, GLdouble y0, GLdouble x1, GLdouble y1);
extern void crStateSelectRects(GLshort x0, GLshort y0, GLshort x1, GLshort y1);
extern void crStateSelectRectfv(const GLfloat *v0, const GLfloat *v1);
extern void crStateSelectRectiv(const GLint *v0, const GLint *v1);
extern void crStateSelectRectdv(const GLdouble *v0, const GLdouble *v1);
extern void crStateSelectRectsv(const GLshort *v0, const GLshort *v1);
extern void crStateSelectBitmap( GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap );
extern void crStateSelectDrawPixels( GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels );
extern void crStateSelectCopyPixels( GLint x, GLint y, GLsizei width, GLsizei height, GLenum type );

#ifdef __cplusplus
}
#endif

#endif /* CR_STATE_FEEDBACK_H */
