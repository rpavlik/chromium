/* Stolen without ceremony from state_tracker/state_client.c and other related files.
 * These versions of the state tracking files do nothing more than update the client
 * state, all of which is passed in by reference.  This allows a utility (like the
 * display list manager) to keep track of important client state required for
 * proper operation without stepping on an SPU that is using the "real" state tracking
 * functions.
 *
 * These functions and their counterparts should be combined into a single set of
 * utilities that can serve both needs, by some programmer doing future enhancements.
 */

#include <stdio.h>
#include <stdarg.h>
#include <GL/gl.h>
#include "cr_mem.h"
#include "dlm.h"
#include "dlm_client.h"
#include "cr_environment.h"
#include "cr_error.h"

#define GLCLIENT_LIST_ALLOC 1024

void crdlmWarning( int line, char *file, GLenum error, char *format, ... )
{
    char errstr[8096];
    va_list args;

    if (crGetenv("CR_DEBUG")) {
	char *glerr;
	va_start( args, format );
	vsprintf( errstr, format, args );
	va_end( args );

	switch (error) {
	    case GL_NO_ERROR:
		glerr = "GL_NO_ERROR";
		break;
	    case GL_INVALID_VALUE:
		glerr = "GL_INVALID_VALUE";
		break;
	    case GL_INVALID_ENUM:
		glerr = "GL_INVALID_ENUM";
		break;
	    case GL_INVALID_OPERATION:
		glerr = "GL_INVALID_OPERATION";
		break;
	    case GL_STACK_OVERFLOW:
		glerr = "GL_STACK_OVERFLOW";
		break;
	    case GL_STACK_UNDERFLOW:
		glerr = "GL_STACK_UNDERFLOW";
		break;
	    case GL_OUT_OF_MEMORY:
		glerr = "GL_OUT_OF_MEMORY";
		break;
	    case GL_TABLE_TOO_LARGE:
		glerr = "GL_TABLE_TOO_LARGE";
		break;
	    default:
		glerr = "unknown";
		break;
	}

	crWarning( "DLM error in %s, line %d: %s: %s\n",
	    file, line, glerr, errstr );
    }
}

/*
 * XXX this is identical to code in state_client.c
 * Try to reuse it.
 */

void crdlmClientInit(CRClientState *c) 
{
    unsigned int i;

    /* pixel pack/unpack */
    c->unpack.rowLength   = 0;
    c->unpack.skipRows    = 0;
    c->unpack.skipPixels  = 0;
    c->unpack.skipImages  = 0;
    c->unpack.alignment   = 4;
    c->unpack.imageHeight = 0;
    c->unpack.swapBytes   = GL_FALSE;
    c->unpack.psLSBFirst  = GL_FALSE;
    c->pack.rowLength     = 0;
    c->pack.skipRows      = 0;
    c->pack.skipPixels    = 0;
    c->pack.skipImages    = 0;
    c->pack.alignment     = 4;
    c->pack.imageHeight   = 0;
    c->pack.swapBytes     = GL_FALSE;
    c->pack.psLSBFirst    = GL_FALSE;

    /* ARB multitexture */
    c->curClientTextureUnit = 0;

    c->list_alloc = GLCLIENT_LIST_ALLOC;
    c->list_size = 0;
    c->list = (int *) crCalloc(c->list_alloc * sizeof (int));

    /* vertex array */
    c->array.v.p = NULL;
    c->array.v.size = 0;
    c->array.v.type = GL_NONE;
    c->array.v.stride = 0;
    c->array.v.enabled = 0;
    c->array.c.p = NULL;
    c->array.c.size = 0;
    c->array.c.type = GL_NONE;
    c->array.c.stride = 0;
    c->array.c.enabled = 0;
    c->array.s.p = NULL;
    c->array.s.size = 0;
    c->array.s.type = GL_NONE;
    c->array.s.stride = 0;
    c->array.s.enabled = 0;
    c->array.e.p = NULL;
    c->array.e.size = 0;
    c->array.e.type = GL_NONE;
    c->array.e.stride = 0;
    c->array.e.enabled = 0;
    c->array.i.p = NULL;
    c->array.i.size = 0;
    c->array.i.type = GL_NONE;
    c->array.i.stride = 0;
    c->array.i.enabled = 0;
    c->array.n.p = NULL;
    c->array.n.size = 0;
    c->array.n.type = GL_NONE;
    c->array.n.stride = 0;
    c->array.n.enabled = 0;
    for (i = 0 ; i < CR_MAX_TEXTURE_UNITS ; i++)
    {
        c->array.t[i].p = NULL;
        c->array.t[i].size = 0;
        c->array.t[i].type = GL_NONE;
        c->array.t[i].stride = 0;
        c->array.t[i].enabled = 0;
    }
#ifdef CR_NV_vertex_program
	for (i = 0; i < CR_MAX_VERTEX_ATTRIBS; i++) {
		c->array.a[i].enabled = GL_FALSE;
		c->array.a[i].type = 0;
		c->array.a[i].size = 0;
		c->array.a[i].stride = 0;
	}
#endif
}


/*
 * PixelStore functions are here, not in state_pixel.c because this
 * is client-side state, like vertex arrays.
 */

void crdlmPixelStoref (GLenum pname, GLfloat param, CRClientState *c)
{

    /* The GL SPEC says I can do this on page 76. */
    switch( pname )
    {
        case GL_PACK_SWAP_BYTES:
        case GL_PACK_LSB_FIRST:
        case GL_UNPACK_SWAP_BYTES:
        case GL_UNPACK_LSB_FIRST:
            crdlmPixelStorei( pname, param == 0.0f ? 0: 1 , c);
            break;
        default:
            crdlmPixelStorei( pname, (GLint) param , c);
            break;
    }
}

void crdlmPixelStorei (GLenum pname, GLint param, CRClientState *c)
{
    switch(pname) {
        case GL_PACK_SWAP_BYTES:
            c->pack.swapBytes = (GLboolean) param;
            break;
        case GL_PACK_LSB_FIRST:
            c->pack.psLSBFirst = (GLboolean) param;
            break;
        case GL_PACK_ROW_LENGTH:
            if (param < 0.0f) 
            {
                crdlmWarning(__LINE__, __FILE__, GL_INVALID_VALUE, "Negative Row Length: %f", param);
                return;
            }
            c->pack.rowLength = param;
            break;
#ifdef CR_OPENGL_VERSION_1_2
        case GL_PACK_IMAGE_HEIGHT:
            if (param < 0.0f) 
            {
                crdlmWarning(__LINE__, __FILE__, GL_INVALID_VALUE, "Negative Image Height: %f", param);
                return;
            }
            c->pack.imageHeight = param;
            break;
#endif
        case GL_PACK_SKIP_PIXELS:
            if (param < 0.0f) 
            {
                crdlmWarning(__LINE__, __FILE__, GL_INVALID_VALUE, "Negative Skip Pixels: %f", param);
                return;
            }
            c->pack.skipPixels = param;
            break;
        case GL_PACK_SKIP_ROWS:
            if (param < 0.0f) 
            {
                crdlmWarning(__LINE__, __FILE__, GL_INVALID_VALUE, "Negative Row Skip: %f", param);
                return;
            }
            c->pack.skipRows = param;
            break;
        case GL_PACK_ALIGNMENT:
            if (((GLint) param) != 1 && 
                    ((GLint) param) != 2 &&
                    ((GLint) param) != 4 &&
                    ((GLint) param) != 8) 
            {
                crdlmWarning(__LINE__, __FILE__, GL_INVALID_VALUE, "Invalid Alignment: %f", param);
                return;
            }
            c->pack.alignment = param;
            break;

        case GL_UNPACK_SWAP_BYTES:
            c->unpack.swapBytes = (GLboolean) param;
            break;
        case GL_UNPACK_LSB_FIRST:
            c->unpack.psLSBFirst = (GLboolean) param;
            break;
        case GL_UNPACK_ROW_LENGTH:
            if (param < 0.0f) 
            {
                crdlmWarning(__LINE__, __FILE__, GL_INVALID_VALUE, "Negative Row Length: %f", param);
                return;
            }
            c->unpack.rowLength = param;
            break;
#ifdef CR_OPENGL_VERSION_1_2
        case GL_UNPACK_IMAGE_HEIGHT:
            if (param < 0.0f) 
            {
                crdlmWarning(__LINE__, __FILE__, GL_INVALID_VALUE, "Negative Image Height: %f", param);
                return;
            }
            c->unpack.imageHeight = param;
            break;
#endif
        case GL_UNPACK_SKIP_PIXELS:
            if (param < 0.0f) 
            {
                crdlmWarning(__LINE__, __FILE__, GL_INVALID_VALUE, "Negative Skip Pixels: %f", param);
                return;
            }
            c->unpack.skipPixels = param;
            break;
        case GL_UNPACK_SKIP_ROWS:
            if (param < 0.0f) 
            {
                crdlmWarning(__LINE__, __FILE__, GL_INVALID_VALUE, "Negative Row Skip: %f", param);
                return;
            }
            c->unpack.skipRows = param;
            break;
        case GL_UNPACK_ALIGNMENT:
            if (((GLint) param) != 1 && 
                    ((GLint) param) != 2 &&
                    ((GLint) param) != 4 &&
                    ((GLint) param) != 8) 
            {
                crdlmWarning(__LINE__, __FILE__, GL_INVALID_VALUE, "Invalid Alignment: %f", param);
                return;
            }
            c->unpack.alignment = param;
            break;
        default:
            crdlmWarning(__LINE__, __FILE__, GL_INVALID_VALUE, "Unknown glPixelStore Pname: %d", pname);
            return;
    }
}

static void setClientState(CRClientState *c, GLenum array, GLboolean state) 
{
    switch (array) 
    {
        case GL_VERTEX_ARRAY:
            c->array.v.enabled = state;
            break;
        case GL_COLOR_ARRAY:
            c->array.c.enabled = state;
            break;
        case GL_NORMAL_ARRAY:
            c->array.n.enabled = state;
            break;
        case GL_INDEX_ARRAY:
            c->array.i.enabled = state;
            break;
        case GL_TEXTURE_COORD_ARRAY:
            c->array.t[c->curClientTextureUnit].enabled = state;
            break;
        case GL_EDGE_FLAG_ARRAY:
            c->array.e.enabled = state;
            break;  
        case GL_SECONDARY_COLOR_ARRAY_EXT:
			c->array.s.enabled = state;
			break;
        default:
            crdlmWarning(__LINE__, __FILE__, GL_INVALID_ENUM, "Invalid Enum passed to Enable/Disable Client State: 0x%x", array );
            return;
    }
}

void crdlmEnableClientState (GLenum array, CRClientState *c) 
{
    setClientState(c, array, GL_TRUE);
}

void crdlmDisableClientState (GLenum array, CRClientState *c) 
{
    setClientState(c, array, GL_FALSE);
}

static void crdlmClientSetPointer (CRClientPointer *cp, GLint size, 
        GLenum type, GLsizei stride, const GLvoid *pointer, CRClientState *c) 
{
    cp->p = (unsigned char *) pointer;
    cp->size = size;
    cp->type = type;
    /* Calculate the bytes per index for address calculation */
    cp->bytesPerIndex = size;
    switch (type) 
    {
        case GL_BYTE:
        case GL_UNSIGNED_BYTE:
            break;
        case GL_SHORT:
        case GL_UNSIGNED_SHORT:
            cp->bytesPerIndex *= sizeof(GLshort);
            break;
        case GL_INT:
        case GL_UNSIGNED_INT:
            cp->bytesPerIndex *= sizeof(GLint);
            break;
        case GL_FLOAT:
            cp->bytesPerIndex *= sizeof(GLfloat);
            break;
        case GL_DOUBLE:
            cp->bytesPerIndex *= sizeof(GLdouble);
            break;
        default:
            crdlmWarning( __LINE__, __FILE__, GL_INVALID_VALUE, "Unknown type of vertex array: %d", type );
            return;
    }

    /* 
     **  Note: If stride==0 then we set the 
     **  stride equal address offset
     **  therefore stride can never equal
     **  zero.
     */
    cp->stride = stride;
    if (!stride) cp->stride = cp->bytesPerIndex;
}

void crdlmVertexPointer(GLint size, GLenum type, 
        GLsizei stride, const GLvoid *p, CRClientState *c) 
{
    if (size != 2 && size != 3 && size != 4)
    {
        crdlmWarning(__LINE__, __FILE__, GL_INVALID_VALUE, "glVertexPointer: invalid size: %d", size);
        return;
    }
    if (type != GL_SHORT && type != GL_INT &&
            type != GL_FLOAT && type != GL_DOUBLE)
    {
        crdlmWarning(__LINE__, __FILE__, GL_INVALID_ENUM, "glVertexPointer: invalid type: %d", type);
        return;
    }
    if (stride < 0) 
    {
        crdlmWarning(__LINE__, __FILE__, GL_INVALID_VALUE, "glVertexPointer: stride was negative: %d", stride);
        return;
    }

    crdlmClientSetPointer(&(c->array.v), size, type, stride, p, c);
}

void crdlmColorPointer(GLint size, GLenum type, 
        GLsizei stride, const GLvoid *p, CRClientState *c) 
{
    if (size != 3 && size != 4)
    {
        crdlmWarning(__LINE__, __FILE__, GL_INVALID_VALUE, "glColorPointer: invalid size: %d", size);
        return;
    }
    if (type != GL_BYTE && type != GL_UNSIGNED_BYTE &&
            type != GL_SHORT && type != GL_UNSIGNED_SHORT &&
            type != GL_INT && type != GL_UNSIGNED_INT &&
            type != GL_FLOAT && type != GL_DOUBLE)
    {
        crdlmWarning(__LINE__, __FILE__, GL_INVALID_ENUM, "glColorPointer: invalid type: %d", type);
        return;
    }
    if (stride < 0) 
    {
        crdlmWarning(__LINE__, __FILE__, GL_INVALID_VALUE, "glColorPointer: stride was negative: %d", stride);
        return;
    }

    crdlmClientSetPointer(&(c->array.c), size, type, stride, p, c);
}

void crdlmSecondaryColorPointerEXT(GLint size,
        GLenum type, GLsizei stride, const GLvoid *p, CRClientState *c)
{
    if (size != 3)
    {
        crdlmWarning(__LINE__, __FILE__, GL_INVALID_VALUE, "glSecondaryColorPointerEXT: invalid size: %d", size);
        return;
    }
    if (type != GL_BYTE && type != GL_UNSIGNED_BYTE &&
            type != GL_SHORT && type != GL_UNSIGNED_SHORT &&
            type != GL_INT && type != GL_UNSIGNED_INT &&
            type != GL_FLOAT && type != GL_DOUBLE)
    {
        crdlmWarning(__LINE__, __FILE__, GL_INVALID_ENUM, "glSecondaryColorPointerEXT: invalid type: %d", type);
        return;
    }
    if (stride < 0) 
    {
        crdlmWarning(__LINE__, __FILE__, GL_INVALID_VALUE, "glSecondaryColorPointerEXT: stride was negative: %d", stride);
        return;
    }

    crdlmClientSetPointer(&(c->array.s), size, type, stride, p, c);
}

void crdlmIndexPointer(GLenum type, GLsizei stride, 
        const GLvoid *p, CRClientState *c) 
{
    if (type != GL_SHORT && type != GL_INT && type != GL_UNSIGNED_BYTE &&
            type != GL_FLOAT && type != GL_DOUBLE)
    {
        crdlmWarning(__LINE__, __FILE__, GL_INVALID_ENUM, "glIndexPointer: invalid type: %d", type);
        return;
    }
    if (stride < 0) 
    {
        crdlmWarning(__LINE__, __FILE__, GL_INVALID_VALUE, "glIndexPointer: stride was negative: %d", stride);
        return;
    }

    crdlmClientSetPointer(&(c->array.i), 1, type, stride, p, c);
}

void crdlmNormalPointer(GLenum type, GLsizei stride, 
        const GLvoid *p, CRClientState *c) 
{
    if (type != GL_BYTE && type != GL_SHORT &&
            type != GL_INT && type != GL_FLOAT &&
            type != GL_DOUBLE)
    {
        crdlmWarning(__LINE__, __FILE__, GL_INVALID_ENUM, "glNormalPointer: invalid type: %d", type);
        return;
    }
    if (stride < 0) 
    {
        crdlmWarning(__LINE__, __FILE__, GL_INVALID_VALUE, "glNormalPointer: stride was negative: %d", stride);
        return;
    }

    crdlmClientSetPointer(&(c->array.n), 3, type, stride, p, c);
}

void crdlmTexCoordPointer(GLint size, GLenum type, 
        GLsizei stride, const GLvoid *p, CRClientState *c) 
{
    if (size != 1 && size != 2 && size != 3 && size != 4)
    {
        crdlmWarning(__LINE__, __FILE__, GL_INVALID_VALUE, "glTexCoordPointer: invalid size: %d", size);
        return;
    }
    if (type != GL_SHORT && type != GL_INT &&
            type != GL_FLOAT && type != GL_DOUBLE)
    {
        crdlmWarning(__LINE__, __FILE__, GL_INVALID_ENUM, "glTexCoordPointer: invalid type: %d", type);
        return;
    }
    if (stride < 0) 
    {
        crdlmWarning(__LINE__, __FILE__, GL_INVALID_VALUE, "glTexCoordPointer: stride was negative: %d", stride);
        return;
    }

    crdlmClientSetPointer(&(c->array.t[c->curClientTextureUnit]), size, type, stride, p, c);
}

void crdlmEdgeFlagPointer(GLsizei stride, const GLvoid *p, CRClientState *c) 
{
    if (stride < 0) 
    {
        crdlmWarning(__LINE__, __FILE__, GL_INVALID_VALUE, "glTexCoordPointer: stride was negative: %d", stride);
        return;
    }

    crdlmClientSetPointer(&(c->array.e), 1, GL_UNSIGNED_BYTE, stride, p, c);
}

void crdlmFogCoordPointerEXT(GLenum type, GLsizei stride, const GLvoid *p, CRClientState *c) 
{
    if (type != GL_BYTE && type != GL_UNSIGNED_BYTE &&
            type != GL_SHORT && type != GL_UNSIGNED_SHORT &&
            type != GL_INT && type != GL_UNSIGNED_INT &&
            type != GL_FLOAT && type != GL_DOUBLE)
    {
        crdlmWarning(__LINE__, __FILE__, GL_INVALID_ENUM, "glFogCoordPointerEXT: invalid type: %d", type);
        return;
    }
    if (stride < 0) 
    {
        crdlmWarning(__LINE__, __FILE__, GL_INVALID_VALUE, "glFogCoordPointerEXT: stride was negative: %d", stride);
        return;
    }

    crdlmClientSetPointer(&(c->array.f), 1, type, stride, p, c);
}

void crdlmVertexAttribPointerNV(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid *p, CRClientState *c) 
{
    if (index > CR_MAX_VERTEX_ATTRIBS)
    {
        crdlmWarning(__LINE__, __FILE__, GL_INVALID_VALUE, "glVertexAttribPointerNV: invalid index: %d", (int) index);
        return;
    }
    if (size != 1 && size != 2 && size != 3 && size != 4)
    {
        crdlmWarning(__LINE__, __FILE__, GL_INVALID_VALUE, "glVertexAttribPointerNV: invalid size: %d", size);
        return;
    }
    if (type != GL_SHORT && type != GL_UNSIGNED_BYTE &&
            type != GL_FLOAT && type != GL_DOUBLE)
    {
        crdlmWarning(__LINE__, __FILE__, GL_INVALID_ENUM, "glVertexAttribPointerNV: invalid type: %d", type);
        return;
    }
    if (stride < 0) 
    {
        crdlmWarning(__LINE__, __FILE__, GL_INVALID_VALUE, "glVertexAttribPointerNV: stride was negative: %d", stride);
        return;
    }

    crdlmClientSetPointer(&(c->array.a[index]), size, type, stride, p, c);
}

void  crdlmClientActiveTextureARB(GLenum texture, CRClientState *c)
{
    c->curClientTextureUnit = texture - GL_TEXTURE0_ARB;
}



/* 
** Currently I treat Interleaved Arrays as if the 
** user uses them as separate arrays.
** Certainly not the most efficient method but it 
** lets me use the same glDrawArrays method.
*/
void crdlmInterleavedArrays(GLenum format, GLsizei stride, const GLvoid *p, CRClientState *c) 
{
    CRClientPointer *cp;
    unsigned char *base = (unsigned char *) p;

    if (stride < 0)
    {
        crdlmWarning(__LINE__, __FILE__, GL_INVALID_OPERATION, "glInterleavedArrays: stride < 0: %d", stride);
        return;
    }

    switch (format) 
    {
        case GL_T4F_C4F_N3F_V4F:
        case GL_T2F_C4F_N3F_V3F:
        case GL_C4F_N3F_V3F:
        case GL_T4F_V4F:
        case GL_T2F_C3F_V3F:
        case GL_T2F_N3F_V3F:
        case GL_C3F_V3F:
        case GL_N3F_V3F:
        case GL_T2F_C4UB_V3F:
        case GL_T2F_V3F:
        case GL_C4UB_V3F:
        case GL_V3F:
        case GL_C4UB_V2F:
        case GL_V2F:
            break;
        default:
            crdlmWarning(__LINE__, __FILE__, GL_INVALID_ENUM, "glInterleavedArrays: Unrecognized format: %d", format);
            return;
    }


/* p, size, type, stride, enabled, bytesPerIndex */
/*
**  VertexPointer 
*/
    
    cp = &(c->array.v);
    cp->type = GL_FLOAT;
    cp->enabled = GL_TRUE;

    switch (format) 
    {
        case GL_T4F_C4F_N3F_V4F:
            cp->p = base+4*sizeof(GLfloat)+4*sizeof(GLfloat)+3*sizeof(GLfloat);
            cp->size = 4;
            break;
        case GL_T2F_C4F_N3F_V3F:
            cp->p = base+2*sizeof(GLfloat)+4*sizeof(GLfloat)+3*sizeof(GLfloat);
            cp->size = 3;
            break;
        case GL_C4F_N3F_V3F:
            cp->p = base+4*sizeof(GLfloat)+3*sizeof(GLfloat);
            cp->size = 3;
            break;
        case GL_T4F_V4F:
            cp->p = base+4*sizeof(GLfloat);
            cp->size = 4;
            break;
        case GL_T2F_C3F_V3F:
            cp->p = base+2*sizeof(GLfloat)+3*sizeof(GLfloat);
            cp->size = 3;
            break;
        case GL_T2F_N3F_V3F:
            cp->p = base+2*sizeof(GLfloat)+3*sizeof(GLfloat);
            cp->size = 3;
            break;
        case GL_C3F_V3F:
            cp->p = base+3*sizeof(GLfloat);
            cp->size = 3;
            break;
        case GL_N3F_V3F:
            cp->p = base+3*sizeof(GLfloat);
            cp->size = 3;
            break;
        case GL_T2F_C4UB_V3F:
            cp->p = base+2*sizeof(GLfloat)+4*sizeof(GLubyte);
            cp->size = 3;
            break;
        case GL_T2F_V3F:
            cp->p = base+2*sizeof(GLfloat);
            cp->size = 3;
            break;
        case GL_C4UB_V3F:
            cp->p = base+4*sizeof(GLubyte);
            cp->size = 3;
            break;
        case GL_V3F:
            cp->p = base;
            cp->size = 3;
            break;
        case GL_C4UB_V2F:
            cp->p = base+4*sizeof(GLubyte);
            cp->size = 2;
            break;
        case GL_V2F:
            cp->p = base;
            cp->size = 2;
            break;
        default:
            crdlmWarning(__LINE__, __FILE__, GL_INVALID_ENUM, "glInterleavedArrays: Unrecognized format: %d", format);
            return;
    }

    cp->bytesPerIndex = cp->size * sizeof (GLfloat);

    if (stride)
        cp->stride = stride + (cp->p - base);
    else
        cp->stride = cp->bytesPerIndex + (cp->p - base);

/*
**  NormalPointer
*/

    cp = &(c->array.n);
    cp->enabled = GL_TRUE;
    switch (format) 
    {
        case GL_T4F_C4F_N3F_V4F:
            cp->p = base+4*sizeof(GLfloat)+4*sizeof(GLfloat);
            cp->stride = 4*sizeof(GLfloat)+4*sizeof(GLfloat)+stride;
            if (!stride) cp->stride += 3*sizeof(GLfloat)+4*sizeof(GLfloat);
            break;
        case GL_T2F_C4F_N3F_V3F:
            cp->p = base+2*sizeof(GLfloat)+4*sizeof(GLfloat);
            cp->stride = 2*sizeof(GLfloat)+4*sizeof(GLfloat)+stride;
            if (!stride) cp->stride += 3*sizeof(GLfloat)+3*sizeof(GLfloat);
            break;
        case GL_C4F_N3F_V3F:
            cp->p = base+4*sizeof(GLfloat);
            cp->stride = 4*sizeof(GLfloat)+stride;
            if (!stride) cp->stride += 3*sizeof(GLfloat)+3*sizeof(GLfloat);
            break;
        case GL_T2F_N3F_V3F:
            cp->p = base+2*sizeof(GLfloat);
            cp->stride = 2*sizeof(GLfloat)+stride;
            if (!stride) cp->stride += 3*sizeof(GLfloat)+3*sizeof(GLfloat);
            break;
        case GL_N3F_V3F:
            cp->p = base;
            cp->stride = stride;
            if (!stride) cp->stride += 3*sizeof(GLfloat)+3*sizeof(GLfloat);
            break;
        case GL_T4F_V4F:
        case GL_T2F_C3F_V3F:
        case GL_C3F_V3F:
        case GL_T2F_C4UB_V3F:
        case GL_T2F_V3F:
        case GL_C4UB_V3F:
        case GL_V3F:
        case GL_C4UB_V2F:
        case GL_V2F:
            cp->enabled = GL_FALSE;
            break;
        default:
            crdlmWarning(__LINE__, __FILE__, GL_INVALID_ENUM, "glInterleavedArrays: Unrecognized format: %d", format);
            return;
    }

    if (cp->enabled) 
    {
        cp->type = GL_FLOAT;
        cp->size = 3;
        cp->bytesPerIndex = cp->size * sizeof (GLfloat);
    }
    
/*
**  ColorPointer
*/

    cp = &(c->array.c);
    cp->enabled = GL_TRUE;
    switch (format) 
    {
        case GL_T4F_C4F_N3F_V4F:
            cp->size = 4;
            cp->type = GL_FLOAT;
            cp->bytesPerIndex = cp->size * sizeof(GLfloat);
            cp->p = base+4*sizeof(GLfloat);
            cp->stride = 4*sizeof(GLfloat)+stride;
            if (!stride) cp->stride += 4*sizeof(GLfloat)+3*sizeof(GLfloat)+4*sizeof(GLfloat);
            break;
        case GL_T2F_C4F_N3F_V3F:
            cp->size = 4;
            cp->type = GL_FLOAT;
            cp->bytesPerIndex = cp->size * sizeof(GLfloat);
            cp->p = base+2*sizeof(GLfloat);
            cp->stride = 2*sizeof(GLfloat)+stride;
            if (!stride) cp->stride += 4*sizeof(GLfloat)+3*sizeof(GLfloat)+3*sizeof(GLfloat);
            break;
        case GL_C4F_N3F_V3F:
            cp->size = 4;
            cp->type = GL_FLOAT;
            cp->bytesPerIndex = cp->size * sizeof(GLfloat);
            cp->p = base;
            cp->stride = stride;
            if (!stride) cp->stride += 4*sizeof(GLfloat)+3*sizeof(GLfloat)+3*sizeof(GLfloat);
            break;
        case GL_T2F_C3F_V3F:
            cp->size = 3;
            cp->type = GL_FLOAT;
            cp->bytesPerIndex = cp->size * sizeof(GLfloat);
            cp->p = base+2*sizeof(GLfloat);
            cp->stride = 2*sizeof(GLfloat)+stride;
            if (!stride) cp->stride += 3*sizeof(GLfloat)+3*sizeof(GLfloat);
            break;
        case GL_C3F_V3F:
            cp->size = 3;
            cp->type = GL_FLOAT;
            cp->bytesPerIndex = cp->size * sizeof(GLfloat);
            cp->p = base;
            cp->stride = stride;
            if (!stride) cp->stride += 3*sizeof(GLfloat) + 3*sizeof(GLfloat);
            break;
        case GL_T2F_C4UB_V3F:
            cp->size = 4;
            cp->type = GL_UNSIGNED_BYTE;
            cp->bytesPerIndex = cp->size * sizeof(GLubyte);
            cp->p = base+2*sizeof(GLfloat);
            cp->stride = 2*sizeof(GLfloat)+stride;
            if (!stride) cp->stride += 4*sizeof(GLubyte)+2*sizeof(GLfloat);
            break;
        case GL_C4UB_V3F:
            cp->size = 4;
            cp->type = GL_UNSIGNED_BYTE;
            cp->bytesPerIndex = cp->size * sizeof(GLubyte);
            cp->p = base;
            cp->stride = stride;
            if (!stride) cp->stride += 4*sizeof(GLubyte)+3*sizeof(GLfloat);
            break;
        case GL_C4UB_V2F:
            cp->size = 4;
            cp->type = GL_UNSIGNED_BYTE;
            cp->bytesPerIndex = cp->size * sizeof(GLubyte);
            cp->p = base;
            cp->stride = stride;
            if (!stride) cp->stride += 4*sizeof(GLubyte)+2*sizeof(GLfloat);
            break;
        case GL_T2F_N3F_V3F:
        case GL_N3F_V3F:
        case GL_T4F_V4F:
        case GL_T2F_V3F:
        case GL_V3F:
        case GL_V2F:
            cp->enabled = GL_FALSE;
            break;
        default:
            crdlmWarning(__LINE__, __FILE__, GL_INVALID_ENUM, "glInterleavedArrays: Unrecognized format: %d", format);
            return;
    }

/*
**  TexturePointer
*/

    cp = &(c->array.t[c->curClientTextureUnit]);
    cp->enabled = GL_TRUE;
    switch (format) 
    {
        case GL_T4F_C4F_N3F_V4F:
            cp->size = 4;
            cp->p = base;
            cp->stride = stride;
            if (!stride) cp->stride = 4*sizeof(GLfloat)+4*sizeof(GLfloat)+3*sizeof(GLfloat)+4*sizeof(GLfloat);
            break;
        case GL_T2F_C4F_N3F_V3F:
            cp->size = 3;
            cp->p = base;
            cp->stride = stride;
            if (!stride) cp->stride = 2*sizeof(GLfloat)+4*sizeof(GLfloat)+3*sizeof(GLfloat)+3*sizeof(GLfloat);
            break;
        case GL_T2F_C3F_V3F:
        case GL_T2F_N3F_V3F:
            cp->size = 3;
            cp->p = base;
            cp->stride = stride;
            if (!stride) cp->stride = 2*sizeof(GLfloat)+3*sizeof(GLfloat)+3*sizeof(GLfloat);
            break;
        case GL_T2F_C4UB_V3F:
            cp->size = 3;
            cp->p = base;
            cp->stride = stride;
            if (!stride) cp->stride = 2*sizeof(GLfloat)+4*sizeof(GLubyte)+3*sizeof(GLfloat);
            break;
        case GL_T4F_V4F:
            cp->size = 4;
            cp->p = base;
            cp->stride = stride;
            if (!stride) cp->stride = 4*sizeof(GLfloat)+4*sizeof(GLfloat);
            break;
        case GL_T2F_V3F:
            cp->size = 3;
            cp->p = base;
            cp->stride = stride;
            if (!stride) cp->stride = 2*sizeof(GLfloat)+3*sizeof(GLfloat);
            break;
        case GL_C4UB_V3F:
        case GL_C4UB_V2F:
        case GL_C3F_V3F:
        case GL_C4F_N3F_V3F:
        case GL_N3F_V3F:
        case GL_V3F:
        case GL_V2F:
            cp->enabled = GL_FALSE;
            break;
        default:
            crdlmWarning(__LINE__, __FILE__, GL_INVALID_ENUM, "glInterleavedArrays: Unrecognized format: %d", format);
            return;
    }

    if (cp->enabled) 
    {
        cp->type = GL_FLOAT;
        cp->bytesPerIndex = cp->size * sizeof (GLfloat);
    }   
}

void crdlmGetPointerv(GLenum pname, GLvoid * * params, CRClientState *c) 
{
    switch (pname) 
    {
        case GL_VERTEX_ARRAY_POINTER:
            *params = (GLvoid *) c->array.v.p;
            break;
        case GL_COLOR_ARRAY_POINTER:
            *params = (GLvoid *) c->array.c.p;
            break;
        case GL_NORMAL_ARRAY_POINTER:
            *params = (GLvoid *) c->array.n.p;
            break;
        case GL_INDEX_ARRAY_POINTER:
            *params = (GLvoid *) c->array.i.p;
            break;
        case GL_TEXTURE_COORD_ARRAY_POINTER:
            *params = (GLvoid *) c->array.t[c->curClientTextureUnit].p;
            break;
        case GL_EDGE_FLAG_ARRAY_POINTER:
            *params = (GLvoid *) c->array.e.p;
            break;
#ifdef CR_EXT_fog_coord
        case GL_FOG_COORDINATE_ARRAY_POINTER_EXT:
            *params = (GLvoid *) c->array.f.p;
            break;
#endif
#ifdef CR_EXT_secondary_color
        case GL_SECONDARY_COLOR_ARRAY_POINTER_EXT:
	    *params = (GLvoid *) c->array.s.p;
	    break;
#endif
        case GL_FEEDBACK_BUFFER_POINTER:
        case GL_SELECTION_BUFFER_POINTER:
            /* do nothing - API switching should pick this up */
            break;
        default:
            crdlmWarning(__LINE__, __FILE__, GL_INVALID_OPERATION,
                    "glGetPointerv: invalid pname: %d", pname);
            return;
    }
}


void crdlmEnableVertexAttribArrayARB (GLenum array, CRClientState *c)
{

}


void crdlmDisableVertexAttribArrayARB (GLenum array, CRClientState *c)
{

}


void crdlmPopClientAttrib( CRClientState *c )
{

}


void crdlmPushClientAttrib( GLbitfield mask, CRClientState *c )
{

}


void crdlmVertexArrayRangeNV( GLsizei length, const GLvoid *pointer, CRClientState *c )
{

}


void crdlmFlushVertexArrayRangeNV( CRClientState *c )
{

}


void crdlmVertexAttribPointerARB( GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer, CRClientState *c )
{

}
