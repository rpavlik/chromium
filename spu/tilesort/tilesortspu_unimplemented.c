/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */
 
#include "tilesortspu.h"
#include "cr_error.h"    

void TILESORTSPU_APIENTRY tilesortspu_BoundsInfo( GLrecti *bounds, GLbyte *payload, GLint len, GLint num_opcodes ) {
	
	(void)bounds;
	(void)payload;
	(void)len;
	(void)num_opcodes;

        crWarning("BoundsInfo not implemented!");
}
 
void TILESORTSPU_APIENTRY tilesortspu_FeedbackBuffer( GLsizei size, GLenum type, GLfloat *buffer ) {

	(void)size;
	(void)type;
	(void)buffer;

        crWarning("FeedbackBuffer not implemented!");
}
 
void TILESORTSPU_APIENTRY tilesortspu_InitNames( void ) {
        crWarning("InitNames not implemented!");
}
 
void TILESORTSPU_APIENTRY tilesortspu_LoadName( GLuint name ) {

	(void)name;

        crWarning("LoadName not implemented!");
}
 
void TILESORTSPU_APIENTRY tilesortspu_PassThrough( GLfloat token ) {

	(void)token;

        crWarning("PassThrough not implemented!");
}
 
void TILESORTSPU_APIENTRY tilesortspu_PopClientAttrib( void ) {
        crWarning("PopClientAttrib not implemented!");
}
 
void TILESORTSPU_APIENTRY tilesortspu_PopName( void ) {
        crWarning("PopName not implemented!");
}
 
void TILESORTSPU_APIENTRY tilesortspu_PushClientAttrib( GLbitfield mask ) {

	(void)mask;

        crWarning("PushClientAttrib not implemented!");
}
 
void TILESORTSPU_APIENTRY tilesortspu_PushName( GLuint name ) {

	(void)name;

        crWarning("PushName not implemented!");
}
 
void TILESORTSPU_APIENTRY tilesortspu_SelectBuffer( GLsizei size, GLuint *buffer ) {

	(void)size;
	(void)buffer;

        crWarning("SelectBuffer not implemented!");
}
 
GLint TILESORTSPU_APIENTRY tilesortspu_RenderMode( GLenum mode ) {

	(void)mode;

        crWarning("RenderMode not implemented!");
	return -1;
}
