/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef CR_STATE_HINT_H 
#define CR_STATE_HINT_H 

#include "cr_glwrapper.h"
#include "state/cr_statetypes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	GLbitvalue dirty;
	GLbitvalue perspectiveCorrection;
	GLbitvalue pointSmooth;
	GLbitvalue lineSmooth;
	GLbitvalue polygonSmooth;
	GLbitvalue fog;
#ifdef CR_EXT_clip_volume_hint
	GLbitvalue clipVolumeClipping;
#endif
#ifdef CR_ARB_texture_compression
	GLbitvalue textureCompression;
#endif
#ifdef CR_SGIS_generate_mipmap
	GLbitvalue generateMipmap;
#endif
} CRHintBits;

typedef struct {
	GLenum perspectiveCorrection;
	GLenum pointSmooth;
	GLenum lineSmooth;
	GLenum polygonSmooth;
	GLenum fog;
#ifdef CR_EXT_clip_volume_hint
	GLenum clipVolumeClipping;
#endif
#ifdef CR_ARB_texture_compression
	GLenum textureCompression;
#endif
#ifdef CR_SGIS_generate_mipmap
	GLenum generateMipmap;
#endif
} CRHintState;

void crStateHintInitBits (CRHintBits *fb);
void crStateHintInit(CRHintState *f);

void crStateHintDiff(CRHintBits *bb, GLbitvalue bitID, 
		CRHintState *from, CRHintState *to);
void crStateHintSwitch(CRHintBits *bb, GLbitvalue bitID, 
		CRHintState *from, CRHintState *to);

#ifdef __cplusplus
}
#endif

#endif /* CR_STATE_HINT_H */
