#include "cr_dlm.h"
#include "cr_mem.h"
#include "cr_pixeldata.h"
#include "cr_string.h"
#include "dlm.h"
#include "dlm_dispatch.h"

/*****************************************************************************
 * These helper functions are used for GL functions that take a pointers,
 * if the size of the arrays that the pointers refer to is not constant.
 * These helper functions will determine, on a case-by-case basis,
 * how much space is needed to store the array.  If the buffer 
 * parameter is not NULL, they will also pack the data into the given
 * array.
 *
 * Many of the functions included deal with pixel state (Bitmap, DrawPixels,
 * etc.).  In all these cases, when the function instance is stored in a
 * display list, its data is read from memory (as per the parameters
 * to PixelStore) and is stored in a tightly packed format (with no
 * excess row length, no pixels skipped, no rows, skipped, and a byte
 * alignment).
 *
 * When the instances are executed again, care must be taken to ensure
 * that the PixelStore client state that unpacks them is set to reflect
 * the tight packing actually used, instead of whatever the current
 * client state indicates.
 *
 * So to do this, client PixelStore state is forced to known values
 * before any instances in the display list are executed.  The client
 * state is then restored to known values afterwards.  (The difficulty
 * of this is somewhat mollified by the observation that PixelStore
 * instances affect client state, and cannot be stored in a display list.)
 *
 */

int crdlm_pointers_Bitmap( struct instanceBitmap *instance, GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap )
{
    unsigned int size = ((int)((width + 7) / 8)) * height;
    /* glBitmap can be called with a NULL size 0 bitmap, say for
     * an empty glyph that only moves the current raster position.
     * crMemcpy will raise an exception with a NULL source pointer, even if
     * the size to copy is 0.  So make sure we don't ram into this.
     * Also, the bitmap isn't necessarily just sitting in memory; the PixelStore
     * client-side state affects how it is read from memory.  It's easiest to just
     * use the utility.
     */
    if (instance && size > 0) {
	CRDLMContextState *state = CURRENT_STATE();
	crBitmapCopy(width, height, instance->bitmap, bitmap,
		&state->clientState->unpack);
    }

    return size;
}

int crdlm_pointers_DrawPixels( struct instanceDrawPixels *instance, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels )
{
    unsigned int size = crImageSize(format, type, width, height);

    if (instance && size > 0) {
	CRDLMContextState *state = CURRENT_STATE();
	crPixelCopy2D(width, height, 
		instance->pixels, format, type, NULL,
		pixels, format, type, &state->clientState->unpack);
    }

    return size;
}
int crdlm_pointers_Fogfv( struct instanceFogfv *instance, GLenum pname, const GLfloat *params )
{
    unsigned int size = (pname == GL_FOG_COLOR?4:1)*sizeof(GLfloat);
    if (instance) crMemcpy(instance->params, params, size);
    return size;
}
int crdlm_pointers_Fogiv( struct instanceFogiv *instance, GLenum pname, const GLint *params )
{
    unsigned int size = (pname == GL_FOG_COLOR?4:1)*sizeof(GLint);
    if (instance) crMemcpy(instance->params, params, size);
    return size;
}
int crdlm_pointers_LightModelfv( struct instanceLightModelfv *instance, GLenum pname, const GLfloat *params )
{
    unsigned int size = (pname == GL_LIGHT_MODEL_AMBIENT?4:1)*sizeof(GLfloat);
    if (instance) crMemcpy(instance->params, params, size);
    return size;
}
int crdlm_pointers_LightModeliv( struct instanceLightModeliv *instance, GLenum pname, const GLint *params )
{
    unsigned int size = (pname == GL_LIGHT_MODEL_AMBIENT?4:1)*sizeof(GLfloat);
    if (instance) crMemcpy(instance->params, params, size);
    return size;
}
int crdlm_pointers_Lightfv( struct instanceLightfv *instance, GLenum light, GLenum pname, const GLfloat *params )
{
    unsigned int size;
    switch(pname) {
	case GL_AMBIENT: case GL_DIFFUSE: case GL_SPECULAR: case GL_POSITION:
	    size = 4 * sizeof(GLfloat);
	    break;
	case GL_SPOT_DIRECTION:
	    size = 3 * sizeof(GLfloat);
	    break;
	default:
	    size = 1 * sizeof(GLfloat);
	    break;
    }
    if (instance) crMemcpy(instance->params, params, size);
    return size;
}
int crdlm_pointers_Lightiv( struct instanceLightiv *instance, GLenum light, GLenum pname, const GLint *params )
{
    unsigned int size;
    switch(pname) {
	case GL_AMBIENT: case GL_DIFFUSE: case GL_SPECULAR: case GL_POSITION:
	    size = 4 * sizeof(GLint);
	    break;
	case GL_SPOT_DIRECTION:
	    size = 3 * sizeof(GLint);
	    break;
	default:
	    size = 1 * sizeof(GLint);
	    break;
    }
    if (instance) crMemcpy(instance->params, params, size);
    return size;
}

/* This utility routine returns the number of components per
 * mapping point for all the glMap* functions.
 */
static int map_num_components(GLenum target)
{
    switch(target) {
	case GL_MAP1_INDEX: case GL_MAP1_TEXTURE_COORD_1:
	    return 1;
	case GL_MAP1_TEXTURE_COORD_2:
	    return 2;
	case GL_MAP1_VERTEX_3: case GL_MAP1_NORMAL: 
	case GL_MAP1_TEXTURE_COORD_3:
	    return 3;
	case GL_MAP1_VERTEX_4: case GL_MAP1_COLOR_4:
	case GL_MAP1_TEXTURE_COORD_4:
	    return 4;

	case GL_MAP2_INDEX: case GL_MAP2_TEXTURE_COORD_1:
	    return 1;
	case GL_MAP2_TEXTURE_COORD_2:
	    return 2;
	case GL_MAP2_VERTEX_3: case GL_MAP2_NORMAL:
	case GL_MAP2_TEXTURE_COORD_3:
	    return 3;
	case GL_MAP2_VERTEX_4: case GL_MAP2_COLOR_4:
	case GL_MAP2_TEXTURE_COORD_4:
	    return 4;
    }
    return 0;
} 


int crdlm_pointers_Map1d( struct instanceMap1d *instance, GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points )
{
    unsigned int numValues = map_num_components(target);
    unsigned int size = order * numValues * sizeof(GLdouble);
    if (instance) {
	/* This one's a little different - we rearrange the order to
	 * compress it, and change the instance's stride value to
	 * match.
	 */
	const GLdouble *src = points;
	GLdouble *dest = instance->points;
	register int i;
	for (i = 0; i < order; i++) {
	    crMemcpy(dest, src, numValues * sizeof(GLdouble));
	    dest += numValues;
	    src += stride;
	}

	/* We override the stride to show we've compressed the data */
	instance->stride = numValues;
    }
    return size;
}
int crdlm_pointers_Map1f( struct instanceMap1f *instance, GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points )
{
    unsigned int numValues = map_num_components(target);
    unsigned int size = order * numValues * sizeof(GLfloat);
    if (instance) {
	/* This one's a little different - we rearrange the order to
	 * compress it, and change the instance's stride value to
	 * match.
	 */
	const GLfloat *src = points;
	GLfloat *dest = instance->points;
	register int i;
	for (i = 0; i < order; i++) {
	    crMemcpy(dest, src, numValues * sizeof(GLfloat));
	    dest += numValues;
	    src += stride;
	}

	/* We override the stride to show we've compressed the data */
	instance->stride = numValues;
    }
    return size;
}
int crdlm_pointers_Map2d( struct instanceMap2d *instance, GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points )
{
    unsigned int numValues = map_num_components(target);
    unsigned int size = uorder * vorder * numValues * sizeof(GLdouble);
    if (instance) {
	register int v, u;
	const GLdouble *src = points;
	GLdouble *dest = instance->points;
	for (v = 0; v < vorder; v++) {
	    for (u = 0; u < uorder; u++) {
		crMemcpy(dest, src, numValues * sizeof(GLdouble));
		dest += numValues;
		src += ustride;
	    }
	    src += vstride - ustride*uorder;
	}
	/* We override the stride to show we've compressed the data */
	instance->ustride = numValues;
	instance->vstride = ustride * uorder;
    }
    return size;
}
int crdlm_pointers_Map2f( struct instanceMap2f *instance, GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points )
{
    unsigned int numValues = map_num_components(target);
    unsigned int size = uorder * vorder * numValues * sizeof(GLfloat);
    if (instance) {
	register int v, u;
	const GLfloat *src = points;
	GLfloat *dest = instance->points;
	for (v = 0; v < vorder; v++) {
	    for (u = 0; u < uorder; u++) {
		crMemcpy(dest, src, numValues * sizeof(GLfloat));
		dest += numValues;
		src += ustride;
	    }
	    src += vstride - ustride*uorder;
	}
	/* We override the stride to show we've compressed the data */
	instance->ustride = numValues;
	instance->vstride = ustride * uorder;
    }
    return size;
}

int crdlm_pointers_Materialfv(struct instanceMaterialfv *instance, GLenum face, GLenum pname, const GLfloat *params)
{
    unsigned int size = 0;
    switch(pname) {
	case GL_AMBIENT_AND_DIFFUSE:
	    size = 8 * sizeof(GLfloat);
	    break;
	case GL_AMBIENT:
	case GL_DIFFUSE:
	case GL_SPECULAR:
	case GL_EMISSION:
	    size = 4 * sizeof(GLfloat);
	    break;
	case GL_SHININESS:
	    size = 1 * sizeof(GLfloat);
	    break;
	case GL_COLOR_INDEXES:
	    size = 3 * sizeof(GLfloat);
	    break;
	default:
	    break;
    }
    if (instance && size > 0) crMemcpy(instance->params, params, size);
    return size;
}

int crdlm_pointers_Materialiv(struct instanceMaterialiv *instance, GLenum face, GLenum pname, const GLint *params)
{
    unsigned int size = 0;
    switch(pname) {
	case GL_AMBIENT_AND_DIFFUSE:
	    size = 8 * sizeof(GLint);
	    break;
	case GL_AMBIENT:
	case GL_DIFFUSE:
	case GL_SPECULAR:
	case GL_EMISSION:
	    size = 4 * sizeof(GLint);
	    break;
	case GL_SHININESS:
	    size = 1 * sizeof(GLint);
	    break;
	case GL_COLOR_INDEXES:
	    size = 3 * sizeof(GLint);
	    break;
	default:
	    break;
    }
    if (instance && size > 0) crMemcpy(instance->params, params, size);
    return size;
}

int crdlm_pointers_PixelMapfv( struct instancePixelMapfv *instance, GLenum map, GLsizei mapsize, const GLfloat *values )
{
    unsigned int size = mapsize * sizeof(GLfloat);
    if (instance && size > 0) crMemcpy(instance->values, values, size);
    return size;
}
int crdlm_pointers_PixelMapuiv( struct instancePixelMapuiv *instance, GLenum map, GLsizei mapsize, const GLuint *values )
{
    unsigned int size = mapsize * sizeof(GLuint);
    if (instance && size > 0) crMemcpy(instance->values, values, size);
    return size;
}
int crdlm_pointers_PixelMapusv( struct instancePixelMapusv *instance, GLenum map, GLsizei mapsize, const GLushort *values )
{
    unsigned int size = mapsize * sizeof(GLushort);
    if (instance && size > 0) crMemcpy(instance->values, values, size);
    return size;
}

int crdlm_pointers_PointParameterfvARB( struct instancePointParameterfvARB *instance, GLenum pname, const GLfloat *params)
{
    unsigned int size = 0;
    switch(pname) {
	case GL_POINT_DISTANCE_ATTENUATION_ARB:
	    size = 3 * sizeof(GLfloat);
	    break;
	default:
	    size = 1 * sizeof(GLfloat);
	    break;
    }
    return size;
}

int crdlm_pointers_PointParameteriv( struct instancePointParameteriv *instance, GLenum pname, const GLint *params)
{
    unsigned int size = 0;
    switch(pname) {
	case GL_POINT_DISTANCE_ATTENUATION_ARB:
	    size = 3 * sizeof(GLint);
	    break;
	default:
	    size = 1 * sizeof(GLint);
	    break;
    }
    return size;
}

int crdlm_pointers_TexEnvfv( struct instanceTexEnvfv *instance, GLenum target, GLenum pname, const GLfloat *params )
{
    unsigned int size = (pname == GL_TEXTURE_ENV_COLOR?4:1)*sizeof(GLfloat);
    if (instance) crMemcpy(instance->params, params, size);
    return size;
}
int crdlm_pointers_TexEnviv( struct instanceTexEnviv *instance, GLenum target, GLenum pname, const GLint *params )
{
    unsigned int size = (pname == GL_TEXTURE_ENV_COLOR?4:1)*sizeof(GLint);
    if (instance) crMemcpy(instance->params, params, size);
    return size;
}
int crdlm_pointers_TexGendv( struct instanceTexGendv *instance, GLenum coord, GLenum pname, const GLdouble *params )
{
    unsigned int size = (pname == GL_OBJECT_PLANE||pname==GL_EYE_PLANE?4:1)*sizeof(GLdouble);
    if (instance) crMemcpy(instance->params, params, size);
    return size;
}
int crdlm_pointers_TexGenfv( struct instanceTexGenfv *instance, GLenum coord, GLenum pname, const GLfloat *params )
{
    unsigned int size = (pname == GL_OBJECT_PLANE||pname==GL_EYE_PLANE?4:1)*sizeof(GLfloat);
    if (instance) crMemcpy(instance->params, params, size);
    return size;
}
int crdlm_pointers_TexGeniv( struct instanceTexGeniv *instance, GLenum coord, GLenum pname, const GLint *params )
{
    unsigned int size = (pname == GL_OBJECT_PLANE||pname==GL_EYE_PLANE?4:1)*sizeof(GLint);
    if (instance) crMemcpy(instance->params, params, size);
    return size;
}
int crdlm_checkpass_TexImage1D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
    /* This function is called before we allocate memory and insert the instance into
     * the display list.  If it returns TRUE (1), the function is instead passed back
     * to the passthrough function, and never enters the display list.  If it returns
     * 0, normal display list processing continues.
     */
    return (target == GL_PROXY_TEXTURE_1D);
}
int crdlm_pointers_TexImage1D( struct instanceTexImage1D *instance, GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels )
{
    unsigned int size = crImageSize(format, type, width, 1);

    if (instance && size > 0) {
	CRDLMContextState *state = CURRENT_STATE();
	crPixelCopy1D(instance->pixels, format, type,
		pixels, format, type, width, &state->clientState->unpack);
    }

    return size;
}
int crdlm_checkpass_CompressedTexImage1DARB(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imagesize, const GLvoid *data)
{
    /* This function is called before we allocate memory and insert the instance into
     * the display list.  If it returns TRUE (1), the function is instead passed back
     * to the passthrough function, and never enters the display list.  If it returns
     * 0, normal display list processing continues.
     */
    return (target == GL_PROXY_TEXTURE_1D);
}
int crdlm_checkpass_TexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
    /* This function is called before we allocate memory and insert the instance into
     * the display list.  If it returns TRUE (1), the function is instead passed back
     * to the passthrough function, and never enters the display list.  If it returns
     * 0, normal display list processing continues.
     */
    return (target == GL_PROXY_TEXTURE_2D);
}
int crdlm_pointers_CompressedTexImage1DARB(struct instanceCompressedTexImage1DARB *instance, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imagesize, const GLvoid *data)
{
    unsigned int size = imagesize;

    if (instance && size > 0) {
	crMemcpy(instance->data, data, size);
    }

    return size;
}

int crdlm_pointers_TexImage2D( struct instanceTexImage2D *instance, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels )
{
    unsigned int size = crImageSize(format, type, width, height);

    if (instance && size > 0) {
	CRDLMContextState *state = CURRENT_STATE();
	crPixelCopy2D(width, height, 
		instance->pixels, format, type, NULL,
		pixels, format, type, &state->clientState->unpack);
    }

    return size;
}
int crdlm_checkpass_CompressedTexImage2DARB(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imagesize, const GLvoid *data)
{
    /* This function is called before we allocate memory and insert the instance into
     * the display list.  If it returns TRUE (1), the function is instead passed back
     * to the passthrough function, and never enters the display list.  If it returns
     * 0, normal display list processing continues.
     */
    return (target == GL_PROXY_TEXTURE_2D);
}
int crdlm_pointers_CompressedTexImage2DARB(struct instanceCompressedTexImage2DARB *instance, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imagesize, const GLvoid *data)
{
    unsigned int size = imagesize;

    if (instance && size > 0) {
	crMemcpy(instance->data, data, size);
    }

    return size;
}

int crdlm_checkpass_TexImage3D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
    /* This function is called before we allocate memory and insert the instance into
     * the display list.  If it returns TRUE (1), the function is instead passed back
     * to the passthrough function, and never enters the display list.  If it returns
     * 0, normal display list processing continues.
     */
    return (target == GL_PROXY_TEXTURE_3D);
}
int crdlm_pointers_TexImage3D( struct instanceTexImage3D *instance, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels )
{
    unsigned int size;
    int is_distrib = ((type == GL_TRUE) || (type == GL_FALSE));

    if (pixels == NULL) {
	size = 0;
    }
    else if (is_distrib) {
	size = crStrlen(pixels) + 1 + (type==GL_TRUE?width*height*3:0);
    }
    else {
	size = crTextureSize(format, type, width, height, depth);
    }

    if (instance && size > 0) {
	if (is_distrib) {
	    crMemcpy(instance->pixels, pixels, size);
	}
	else {
	    CRDLMContextState *state = CURRENT_STATE();
	    crPixelCopy3D(width, height, depth,
		instance->pixels, format, type, NULL,
		pixels, format, type, &state->clientState->unpack);
	}
    }

    return size;
}
int crdlm_checkpass_TexImage3DEXT(GLenum target, GLint level, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
    /* This function is called before we allocate memory and insert the instance into
     * the display list.  If it returns TRUE (1), the function is instead passed back
     * to the passthrough function, and never enters the display list.  If it returns
     * 0, normal display list processing continues.
     */
    return (target == GL_PROXY_TEXTURE_3D);
}
int crdlm_pointers_TexImage3DEXT( struct instanceTexImage3DEXT *instance, GLenum target, GLint level, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels )
{
    unsigned int size;
    int is_distrib = ((type == GL_TRUE) || (type == GL_FALSE));

    if (pixels == NULL) {
	size = 0;
    }
    else if (is_distrib) {
	size = crStrlen(pixels) + 1 + (type==GL_TRUE?width*height*3:0);
    }
    else {
	size = crTextureSize(format, type, width, height, depth);
    }

    if (instance && size > 0) {
	if (is_distrib) {
	    crMemcpy(instance->pixels, pixels, size);
	}
	else {
	    CRDLMContextState *state = CURRENT_STATE();
	    crPixelCopy3D(width, height, depth,
		instance->pixels, format, type, NULL,
		pixels, format, type, &state->clientState->unpack);
	}
    }

    return size;
}

int crdlm_checkpass_CompressedTexImage3DARB(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imagesize, const GLvoid *data)
{
    /* This function is called before we allocate memory and insert the instance into
     * the display list.  If it returns TRUE (1), the function is instead passed back
     * to the passthrough function, and never enters the display list.  If it returns
     * 0, normal display list processing continues.
     */
    return (target == GL_PROXY_TEXTURE_3D);
}
int crdlm_pointers_CompressedTexImage3DARB(struct instanceCompressedTexImage3DARB *instance, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imagesize, const GLvoid *data)
{
    unsigned int size = imagesize;

    if (instance && size > 0) {
	crMemcpy(instance->data, data, size);
    }

    return size;
}

int crdlm_pointers_TexParameterfv( struct instanceTexParameterfv *instance, GLenum target, GLenum pname, const GLfloat *params )
{
    unsigned int size = (pname == GL_TEXTURE_BORDER_COLOR?4:1)*sizeof(GLfloat);
    if (instance) crMemcpy(instance->params, params, size);
    return size;
}
int crdlm_pointers_TexParameteriv( struct instanceTexParameteriv *instance, GLenum target, GLenum pname, const GLint *params )
{
    unsigned int size = (pname == GL_TEXTURE_BORDER_COLOR?4:1)*sizeof(GLfloat);
    if (instance) crMemcpy(instance->params, params, size);
    return size;
}
int crdlm_pointers_TexSubImage1D( struct instanceTexSubImage1D *instance, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels )
{
    unsigned int size = crImageSize(format, type, width, 1);

    if (instance && size > 0) {
	CRDLMContextState *state = CURRENT_STATE();
	crPixelCopy1D(instance->pixels, format, type,
		pixels, format, type, width, &state->clientState->unpack);
    }

    return size;
}
int crdlm_pointers_TexSubImage2D( struct instanceTexSubImage2D *instance, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels )
{
    unsigned int size = crImageSize(format, type, width, height);

    if (instance && size > 0) {
	CRDLMContextState *state = CURRENT_STATE();
	crPixelCopy2D(width, height, 
		instance->pixels, format, type, NULL,
		pixels, format, type, &state->clientState->unpack);
    }

    return size;
}
int crdlm_pointers_TexSubImage3D( struct instanceTexSubImage3D *instance, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels )
{
    unsigned int size;
    int is_distrib = ((type == GL_TRUE) || (type == GL_FALSE));

    if (pixels == NULL) {
	size = 0;
    }
    else if (is_distrib) {
	size = crStrlen(pixels) + 1 + (type==GL_TRUE?width*height*3:0);
    }
    else {
	size = crTextureSize(format, type, width, height, depth);
    }

    if (instance && size > 0) {
	if (is_distrib) {
	    crMemcpy(instance->pixels, pixels, size);
	}
	else {
	    CRDLMContextState *state = CURRENT_STATE();
	    crPixelCopy3D(width, height, depth,
		instance->pixels, format, type, NULL,
		pixels, format, type, &state->clientState->unpack);
	}
    }

    return size;
}

int crdlm_pointers_CompressedTexSubImage1DARB(struct instanceCompressedTexSubImage1DARB *instance, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imagesize, const GLvoid *data)
{
    unsigned int size = imagesize;

    if (instance && size > 0) {
	crMemcpy(instance->data, data, size);
    }

    return size;
}

int crdlm_pointers_CompressedTexSubImage2DARB(struct instanceCompressedTexSubImage2DARB *instance, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imagesize, const GLvoid *data)
{
    unsigned int size = imagesize;

    if (instance && size > 0) {
	crMemcpy(instance->data, data, size);
    }

    return size;
}
int crdlm_pointers_CompressedTexSubImage3DARB(struct instanceCompressedTexSubImage3DARB *instance, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imagesize, const GLvoid *data)
{
    unsigned int size = imagesize;

    if (instance && size > 0) {
	crMemcpy(instance->data, data, size);
    }

    return size;
}

int crdlm_pointers_Rectdv(struct instanceRectdv *instance, const GLdouble *v1, const GLdouble *v2)
{
    unsigned int size = 4 * sizeof(GLdouble);
    if (instance) {
	instance->data[0] = v1[0];
	instance->data[1] = v1[1];
	instance->data[2] = v2[0];
	instance->data[3] = v2[1];
	instance->v1 = &instance->data[0];
	instance->v2 = &instance->data[2];
    }
    return size;
}
int crdlm_pointers_Rectfv(struct instanceRectfv *instance, const GLfloat *v1, const GLfloat *v2)
{
    unsigned int size = 4 * sizeof(GLfloat);
    if (instance) {
	instance->data[0] = v1[0];
	instance->data[1] = v1[1];
	instance->data[2] = v2[0];
	instance->data[3] = v2[1];
	instance->v1 = &instance->data[0];
	instance->v2 = &instance->data[2];
    }
    return size;
}
int crdlm_pointers_Rectiv(struct instanceRectiv *instance, const GLint *v1, const GLint *v2)
{
    unsigned int size = 4 * sizeof(GLint);
    if (instance) {
	instance->data[0] = v1[0];
	instance->data[1] = v1[1];
	instance->data[2] = v2[0];
	instance->data[3] = v2[1];
	instance->v1 = &instance->data[0];
	instance->v2 = &instance->data[2];
    }
    return size;
}
int crdlm_pointers_Rectsv(struct instanceRectsv *instance, const GLshort *v1, const GLshort *v2)
{
    unsigned int size = 4 * sizeof(GLshort);
    if (instance) {
	instance->data[0] = v1[0];
	instance->data[1] = v1[1];
	instance->data[2] = v2[0];
	instance->data[3] = v2[1];
	instance->v1 = &instance->data[0];
	instance->v2 = &instance->data[2];
    }
    return size;
}

int crdlm_pointers_PrioritizeTextures(struct instancePrioritizeTextures *instance, GLsizei n, const GLuint *textures, const GLclampf *priorities)
{
    unsigned int size = n * (sizeof(GLuint) + sizeof(GLclampf));
    if (instance) {
	instance->textures = (GLuint *)&instance->data[0];
	instance->priorities = (GLclampf *)(((char *)&instance->data[0]) + n * sizeof(GLuint));
	if (size > 0) {
	    crMemcpy(instance->textures, textures, n * sizeof(GLuint));
	    crMemcpy(instance->priorities, priorities, n * sizeof(GLclampf));
	}
    }

    return size;
}

static int combiner_num_components(GLenum pname)
{
    switch(pname) {
	case GL_CONSTANT_COLOR0_NV:
	case GL_CONSTANT_COLOR1_NV:
	    return 4;
	case GL_NUM_GENERAL_COMBINERS_NV:
	case GL_COLOR_SUM_CLAMP_NV:
	    return 1;
    }
    return 0;
} 
int crdlm_pointers_CombinerParameterivNV(struct instanceCombinerParameterivNV *instance, GLenum pname, const GLint *params)
{
    unsigned int size = combiner_num_components(pname) * sizeof(GLint);
    if (instance && size > 0) crMemcpy(instance->params, params, size);
    return size;
}
int crdlm_pointers_CombinerParameterfvNV(struct instanceCombinerParameterfvNV *instance, GLenum pname, const GLfloat *params)
{
    unsigned int size = combiner_num_components(pname) * sizeof(GLfloat);
    if (instance && size > 0) crMemcpy(instance->params, params, size);
    return size;
}

static int combinerstage_num_components(GLenum pname)
{
    switch(pname) {
	case GL_CONSTANT_COLOR0_NV:
	case GL_CONSTANT_COLOR1_NV:
	    return 4;
    }
    return 0;
} 
int crdlm_pointers_CombinerStageParameterfvNV(struct instanceCombinerStageParameterfvNV *instance, GLenum stage, GLenum pname, const GLfloat *params)
{
    unsigned int size = combinerstage_num_components(pname) * sizeof(GLfloat);
    if (instance && size > 0) crMemcpy(instance->params, params, size);
    return size;
}

static int program_num_components(GLenum target)
{
    switch(target) {
	case GL_VERTEX_STATE_PROGRAM_NV:
	    return 4;
    }
    return 0;
} 
int crdlm_pointers_ExecuteProgramNV(struct instanceExecuteProgramNV *instance, GLenum target, GLuint id, const GLfloat *params)
{
    unsigned int size = program_num_components(target) * sizeof(GLfloat);
    if (instance && size > 0) crMemcpy(instance->params, params, size);
    return size;
}

int crdlm_pointers_RequestResidentProgramsNV(struct instanceRequestResidentProgramsNV *instance, GLsizei n, const GLuint *ids)
{
    unsigned int size = 4*sizeof(GLuint);
    if (instance && size > 0) crMemcpy(instance->ids, ids, size);
    return size;
}

int crdlm_pointers_LoadProgramNV(struct instanceLoadProgramNV *instance, GLenum target, GLuint id, GLsizei len, const GLubyte *program)
{
    unsigned int size = len*sizeof(GLubyte);
    if (instance && size > 0) crMemcpy(instance->program, program, size);
    return size;
}

int crdlm_pointers_ProgramNamedParameter4dNV(struct instanceProgramNamedParameter4dNV *instance, GLuint id, GLsizei len, const GLubyte * name, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
	unsigned int size = len * sizeof(GLubyte);
	/* XXX */
	return size;
}

int crdlm_pointers_ProgramNamedParameter4dvNV(struct instanceProgramNamedParameter4dvNV *instance, GLuint id, GLsizei len, const GLubyte * name, const GLdouble * v)
{
	unsigned int size = len * sizeof(GLubyte);
	/* XXX */
	return size;
}
	
int crdlm_pointers_ProgramNamedParameter4fNV(struct instanceProgramNamedParameter4fNV *instance, GLuint id, GLsizei len, const GLubyte * name, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	unsigned int size = len * sizeof(GLubyte);
	/* XXX */
	return size;
}

int crdlm_pointers_ProgramNamedParameter4fvNV(struct instanceProgramNamedParameter4fvNV *instance, GLuint id, GLsizei len, const GLubyte * name, const GLfloat * v)
{
	unsigned int size = len * sizeof(GLubyte);
	/* XXX */
	return size;
}

int crdlm_pointers_ProgramStringARB(struct instanceProgramStringARB *instance, GLenum target, GLenum format, GLsizei len, const GLvoid * string)
{
	unsigned int size = len*sizeof(GLubyte);
	if (instance && size > 0) crMemcpy(instance->string, string, size);
	return size;
}

int crdlm_pointers_CallLists(struct instanceCallLists *instance, GLsizei n, GLenum type, const GLvoid *lists )
{
	unsigned int size;
	switch (type) {
		case GL_BYTE:
			size = sizeof(GLbyte);
			break;
		case GL_UNSIGNED_BYTE:
			size = sizeof(GLubyte);
			break;
		case GL_SHORT:
			size = sizeof(GLshort);
			break;
		case GL_UNSIGNED_SHORT:
			size = sizeof(GLushort);
			break;
		case GL_INT:
			size = sizeof(GLint);
			break;
		case GL_UNSIGNED_INT:
			size = sizeof(GLuint);
			break;
		case GL_FLOAT:
			size = sizeof(GLfloat);
			break;
		case GL_2_BYTES:
			size = 2 * sizeof(GLbyte);
			break;
		case GL_3_BYTES:
			size = 3 * sizeof(GLbyte);
			break;
		case GL_4_BYTES:
			size = 4 * sizeof(GLbyte);
			break;
		default:
			size = 0;
	}
	size *= n;
	if (instance && size > 0) crMemcpy(instance->lists, lists, size);
	return size;
}


int crdlm_pointers_VertexAttribs1dvNV(struct instanceVertexAttribs1dvNV *instance, GLuint index, GLsizei n, const GLdouble *v)
{
   return 1 * n * sizeof(GLdouble);
}

int crdlm_pointers_VertexAttribs1fvNV(struct instanceVertexAttribs1fvNV *instance, GLuint index, GLsizei n, const GLfloat *v)
{
   return 1 * n * sizeof(GLfloat);
}

int crdlm_pointers_VertexAttribs1svNV(struct instanceVertexAttribs1svNV *instance, GLuint index, GLsizei n, const GLshort *v)
{
   return 1 * n * sizeof(GLshort);
}

int crdlm_pointers_VertexAttribs2dvNV(struct instanceVertexAttribs2dvNV *instance, GLuint index, GLsizei n, const GLdouble *v)
{
   return 2 * n * sizeof(GLdouble);
}

int crdlm_pointers_VertexAttribs2fvNV(struct instanceVertexAttribs2fvNV *instance, GLuint index, GLsizei n, const GLfloat *v)
{
   return 2 * n * sizeof(GLfloat);
}

int crdlm_pointers_VertexAttribs2svNV(struct instanceVertexAttribs2svNV *instance, GLuint index, GLsizei n, const GLshort *v)
{
   return 2 * n * sizeof(GLshort);
}

int crdlm_pointers_VertexAttribs3dvNV(struct instanceVertexAttribs3dvNV *instance, GLuint index, GLsizei n, const GLdouble *v)
{
   return 3 * n * sizeof(GLdouble);
}

int crdlm_pointers_VertexAttribs3fvNV(struct instanceVertexAttribs3fvNV *instance, GLuint index, GLsizei n, const GLfloat *v)
{
   return 3 * n * sizeof(GLfloat);
}

int crdlm_pointers_VertexAttribs3svNV(struct instanceVertexAttribs3svNV *instance, GLuint index, GLsizei n, const GLshort *v)
{
   return 3 * n * sizeof(GLshort);
}

int crdlm_pointers_VertexAttribs4dvNV(struct instanceVertexAttribs4dvNV *instance, GLuint index, GLsizei n, const GLdouble *v)
{
   return 4 * n * sizeof(GLdouble);
}

int crdlm_pointers_VertexAttribs4fvNV(struct instanceVertexAttribs4fvNV *instance, GLuint index, GLsizei n, const GLfloat *v)
{
   return 4 * n * sizeof(GLfloat);
}

int crdlm_pointers_VertexAttribs4svNV(struct instanceVertexAttribs4svNV *instance, GLuint index, GLsizei n, const GLshort *v)
{
   return 4 * n * sizeof(GLshort);
}

int crdlm_pointers_VertexAttribs4ubvNV(struct instanceVertexAttribs4ubvNV *instance, GLuint index, GLsizei n, const GLubyte *v)
{
   return 4 * n * sizeof(GLubyte);
}

int crdlm_pointers_ZPixCR( struct instanceZPixCR *instance, GLsizei width, 
			 GLsizei height, GLenum format, GLenum type, 
			 GLenum ztype, GLint zparm, GLint length, 
			 const GLvoid *pixels )
{
     unsigned int size = length;
     if (instance && size > 0) {
	  crMemcpy(instance->pixels,pixels,length);
     }
     
     return size;
}
