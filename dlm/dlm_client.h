#ifndef _DLM_CLIENT_H

#include <GL/gl.h>
#include "state/cr_client.h"

extern void crdlmClientInit(CRClientState *c);
extern void crdlmEnableClientState(GLenum array, CRClientState *c);
extern void crdlmDisableClientState(GLenum array, CRClientState *c);
extern void crdlmPixelStoref(GLenum pname, GLfloat param, CRClientState *c);
extern void crdlmPixelStorei(GLenum pname, GLint param, CRClientState *c);
extern void crdlmVertexPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *p, CRClientState *c);
extern void crdlmColorPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *p, CRClientState *c);
extern void crdlmSecondaryColorPointerEXT(GLint size, GLenum type, GLsizei stride, const GLvoid *p, CRClientState *c);
extern void crdlmIndexPointer(GLenum type, GLsizei stride, const GLvoid *p, CRClientState *c);
extern void crdlmNormalPointer(GLenum type, GLsizei stride, const GLvoid *p, CRClientState *c);
extern void crdlmTexCoordPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *p, CRClientState *c);
extern void crdlmEdgeFlagPointer(GLsizei stride, const GLvoid *p, CRClientState *c);
extern void crdlmFogCoordPointerEXT(GLenum type, GLsizei stride, const GLvoid *p, CRClientState *c);
extern void crdlmVertexAttribPointerNV(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid *p, CRClientState *c);
extern void crdlmInterleavedArrays(GLenum format, GLsizei stride, const GLvoid *p, CRClientState *c);
extern void crdlmGetPointerv(GLenum pname, GLvoid * * params, CRClientState *c);
extern void crdlmClientActiveTextureARB(GLenum texture, CRClientState *c);
extern void crdlmEnableVertexAttribArrayARB (GLenum array, CRClientState *c);
extern void crdlmDisableVertexAttribArrayARB (GLenum array, CRClientState *c);
extern void crdlmPopClientAttrib( CRClientState *c );
extern void crdlmPushClientAttrib( GLbitfield mask, CRClientState *c );
extern void crdlmVertexArrayRangeNV( GLsizei length, const GLvoid *pointer, CRClientState *c );
extern void crdlmFlushVertexArrayRangeNV( CRClientState *c );
extern void crdlmVertexAttribPointerARB( GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer, CRClientState *c );

#endif
