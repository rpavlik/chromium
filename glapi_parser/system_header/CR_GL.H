/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */
void glBarrierCreateCR (GLuint name, GLuint count);
void glBarrierDestroyCR (GLuint name);
void glBarrierExecCR (GLuint name);
void glSemaphoreCreateCR (GLuint name, GLuint count);
void glSemaphoreDestroyCR (GLuint name);
void glSemaphorePCR (GLuint name);
void glSemaphoreVCR (GLuint name);
void glBoundsInfoCR (const GLrecti *bounds, const GLbyte *payload, GLint len, GLint num_opcodes);
void glBlendColorEXT (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
void glBlendEquationEXT (GLenum mode);
void glMultiTexCoord1dARB (GLenum texture, GLdouble s);
void glMultiTexCoord1fARB (GLenum texture, GLfloat s);
void glMultiTexCoord1iARB (GLenum texture, GLint s);
void glMultiTexCoord1sARB (GLenum texture, GLshort s);
void glMultiTexCoord2dARB (GLenum texture, GLdouble s, GLdouble t);
void glMultiTexCoord2fARB (GLenum texture, GLfloat s, GLfloat t);
void glMultiTexCoord2iARB (GLenum texture, GLint s, GLint t);
void glMultiTexCoord2sARB (GLenum texture, GLshort s, GLshort t);
void glMultiTexCoord3dARB (GLenum texture, GLdouble s, GLdouble t, GLdouble r);
void glMultiTexCoord3fARB (GLenum texture, GLfloat s, GLfloat t, GLfloat r);
void glMultiTexCoord3iARB (GLenum texture, GLint s, GLint t, GLint r);
void glMultiTexCoord3sARB (GLenum texture, GLshort s, GLshort t, GLshort r);
void glMultiTexCoord4dARB (GLenum texture, GLdouble s, GLdouble t, GLdouble r, GLdouble q);
void glMultiTexCoord4fARB (GLenum texture, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
void glMultiTexCoord4iARB (GLenum texture, GLint s, GLint t, GLint r, GLint q);
void glMultiTexCoord4sARB (GLenum texture, GLshort s, GLshort t, GLshort r, GLshort q);
void glMultiTexCoord1dvARB (GLenum texture, const GLdouble *t);
void glMultiTexCoord1fvARB (GLenum texture, const GLfloat *t);
void glMultiTexCoord1ivARB (GLenum texture, const GLint *t);
void glMultiTexCoord1svARB (GLenum texture, const GLshort *t);
void glMultiTexCoord2dvARB (GLenum texture, const GLdouble *t);
void glMultiTexCoord2fvARB (GLenum texture, const GLfloat *t);
void glMultiTexCoord2ivARB (GLenum texture, const GLint *t);
void glMultiTexCoord2svARB (GLenum texture, const GLshort *t);
void glMultiTexCoord3dvARB (GLenum texture, const GLdouble *t);
void glMultiTexCoord3fvARB (GLenum texture, const GLfloat *t);
void glMultiTexCoord3ivARB (GLenum texture, const GLint *t);
void glMultiTexCoord3svARB (GLenum texture, const GLshort *t);
void glMultiTexCoord4dvARB (GLenum texture, const GLdouble *t);
void glMultiTexCoord4fvARB (GLenum texture, const GLfloat *t);
void glMultiTexCoord4ivARB (GLenum texture, const GLint *t);
void glMultiTexCoord4svARB (GLenum texture, const GLshort *t);
void glActiveTextureARB (GLenum texture);
void glClientActiveTextureARB (GLenum texture);
/* GL_NV_register_combiners */
void glCombinerParameterfvNV (GLenum pname, const GLfloat *params);
void glCombinerParameterivNV (GLenum pname, const GLint *params);
void glCombinerParameterfNV (GLenum pname, GLfloat param);
void glCombinerParameteriNV (GLenum pname, GLint param);
void glCombinerInputNV (GLenum stage, GLenum portion, GLenum variable, GLenum input, GLenum mapping, GLenum componentUsage);
void glCombinerOutputNV (GLenum stage, GLenum portion, GLenum abOutput, GLenum cdOutput, GLenum sumOutput, GLenum scale, GLenum bias, GLboolean abDotProduct, GLboolean cdDotProduct, GLboolean muxSum);
void glFinalCombinerInputNV (GLenum variable, GLenum input, GLenum mapping, GLenum componentUsage);
void glGetCombinerInputParameterfvNV (GLenum stage, GLenum portion, GLenum variable, GLenum pname, GLfloat *params);
void glGetCombinerInputParameterivNV (GLenum stage, GLenum portion, GLenum variable, GLenum pname, GLint *params);
void glGetCombinerOutputParameterfvNV (GLenum stage, GLenum portion, GLenum pname, GLfloat *params);
void glGetCombinerOutputParameterivNV (GLenum stage, GLenum portion, GLenum pname, GLint *params);
void glGetFinalCombinerInputParameterfvNV (GLenum variable, GLenum pname, GLfloat *params);
void glGetFinalCombinerInputParameterivNV (GLenum variable, GLenum pname, GLint *params);
/* GL_NV_register_combiners2 */
void glCombinerStageParameterfvNV (GLenum stage, GLenum pname, const GLfloat *params);
void glGetCombinerStageParameterfvNV (GLenum stage, GLenum pname, GLfloat *params);
/* GL_EXT_secondary_color */
void glSecondaryColor3bEXT (GLbyte red, GLbyte green, GLbyte blue);
void glSecondaryColor3bvEXT (const GLbyte *v);
void glSecondaryColor3dEXT (GLdouble red, GLdouble green, GLdouble blue);
void glSecondaryColor3dvEXT (const GLdouble *v);
void glSecondaryColor3fEXT (GLfloat red, GLfloat green, GLfloat blue);
void glSecondaryColor3fvEXT (const GLfloat *v);
void glSecondaryColor3iEXT (GLint red, GLint green, GLint blue);
void glSecondaryColor3ivEXT (const GLint *v);
void glSecondaryColor3sEXT (GLshort red, GLshort green, GLshort blue);
void glSecondaryColor3svEXT (const GLshort *v);
void glSecondaryColor3ubEXT (GLubyte red, GLubyte green, GLubyte blue);
void glSecondaryColor3ubvEXT (const GLubyte *v);
void glSecondaryColor3uiEXT (GLuint red, GLuint green, GLuint blue);
void glSecondaryColor3uivEXT (const GLuint *v);
void glSecondaryColor3usEXT (GLushort red, GLushort green, GLushort blue);
void glSecondaryColor3usvEXT (const GLushort *v);
void glSecondaryColorPointerEXT (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
void glChromiumParameteriCR (GLenum target, GLint value);
void glChromiumParameterfCR (GLenum target, GLfloat value);
void glChromiumParametervCR (GLenum target, GLenum type, GLsizei count, const GLvoid *values);
void glGetChromiumParametervCR (GLenum target, GLuint index, GLenum type, GLsizei count, GLvoid *values);
