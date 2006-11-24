/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef CR_PACK_BBOX_H
#define CR_PACK_BBOX_H

#include "cr_pack.h"

#define UPDATE_1D_BBOX(X) \
if (pc->bounds_min.x > (GLfloat) (X))   pc->bounds_min.x = (GLfloat) (X);\
if (pc->bounds_min.y > 0.0f)  pc->bounds_min.y = 0.0f;\
if (pc->bounds_min.z > 0.0f)  pc->bounds_min.z = 0.0f;\
if (pc->bounds_max.x < (GLfloat) (X))   pc->bounds_max.x = (GLfloat) (X);\
if (pc->bounds_max.y < 0.0f)  pc->bounds_max.y = 0.0f;\
if (pc->bounds_max.z < 0.0f)  pc->bounds_max.z = 0.0f

#define UPDATE_2D_BBOX(X, Y) \
if (pc->bounds_min.x > (GLfloat) (X))   pc->bounds_min.x = (GLfloat) (X);\
if (pc->bounds_min.y > (GLfloat) (Y))   pc->bounds_min.y = (GLfloat) (Y);\
if (pc->bounds_min.z > 0.0f)  pc->bounds_min.z = 0.0f;\
if (pc->bounds_max.x < (GLfloat) (X))   pc->bounds_max.x = (GLfloat) (X);\
if (pc->bounds_max.y < (GLfloat) (Y))   pc->bounds_max.y = (GLfloat) (Y);\
if (pc->bounds_max.z < 0.0f)  pc->bounds_max.z = 0.0f

#define UPDATE_3D_BBOX(X, Y, Z) \
if (pc->bounds_min.x > (GLfloat) (X))   pc->bounds_min.x = (GLfloat) (X); \
if (pc->bounds_min.y > (GLfloat) (Y))   pc->bounds_min.y = (GLfloat) (Y); \
if (pc->bounds_min.z > (GLfloat) (Z))   pc->bounds_min.z = (GLfloat) (Z); \
if (pc->bounds_max.x < (GLfloat) (X))   pc->bounds_max.x = (GLfloat) (X); \
if (pc->bounds_max.y < (GLfloat) (Y))   pc->bounds_max.y = (GLfloat) (Y); \
if (pc->bounds_max.z < (GLfloat) (Z))   pc->bounds_max.z = (GLfloat) (Z)

#define UPDATE_4D_BBOX UPDATE_3D_BBOX

#ifdef SIMD
/* These appear to have bit-rotted a long time ago */
#define UPDATE_3D_BBOX_SIMD(X) \
__asm {\
	__asm movups xmm0, (X)\
	__asm movaps xmm1, pc->bounds_min.x\
	__asm movaps xmm2, pc->bounds_max.x\
	__asm minps xmm1, xmm0\
	__asm maxps xmm2, xmm0\
	__asm movaps pc->bounds_min.x, xmm1\
	__asm movaps pc->bounds_max.x, xmm2\
}
#define UPDATE_3D_BBOX_SIMD_PACK(X, Y, Z) \
__asm {\
	__asm mov ecx, [data_ptr]\
	__asm movups xmm0, (X)\
	__asm movaps xmm1, pc->bounds_min.x\
	__asm movaps xmm2, pc->bounds_max.x\
	__asm minps xmm1, xmm0\
	__asm maxps xmm2, xmm0\
	__asm movaps pc->bounds_min.x, xmm1\
	__asm movaps pc->bounds_max.x, xmm2\
	__asm movups [ecx], xmm0\
}
#define UPDATE_3DV_BBOX_SIMD(V) \
__asm {\
	__asm mov eax, [(V)]\
	__asm mov ecx, [data_ptr]\
	__asm movups xmm0, [eax]\
	__asm movaps xmm1, pc->bounds_min.x\
	__asm movaps xmm2, pc->bounds_max.x\
	__asm minps xmm1, xmm0\
	__asm maxps xmm2, xmm0\
	__asm movaps pc->bounds_min.x, xmm1\
	__asm movaps pc->bounds_max.x, xmm2\
	__asm movups [ecx], xmm0\
}
#else
#define UPDATE_3DV_BBOX_SIMD() { UPDATE_WITH_3D_VFLOATS(); UPDATE_3D_BBOX();}
#define UPDATE_3D_BBOX_SIMD(X,Y,Z)  UPDATE_3D_BBOX(X, Y, Z)
#endif

#define UPDATE_WITH_1D_FLOATS() \
	UPDATE_1D_BBOX(x)

#define UPDATE_WITH_2D_FLOATS() \
	UPDATE_2D_BBOX(x, y)

#define UPDATE_WITH_3D_FLOATS() \
	UPDATE_3D_BBOX(x, y, z)

#define UPDATE_WITH_4D_FLOATS() \
	GLfloat fx = (GLfloat) x; \
	GLfloat fy = (GLfloat) y; \
	GLfloat fz = (GLfloat) z; \
	GLfloat fw = (GLfloat) w; \
	fx /= fw; fy /= fw; fz/= fw; \
	UPDATE_3D_BBOX(fx, fy, fz)

/* For glVertexAttrib4N*ARB */
#define UPDATE_WITH_3D_FLOATS_B_NORMALIZED() \
	GLfloat fx = (GLfloat) x * (1.0 / 128.0); \
	GLfloat fy = (GLfloat) y * (1.0 / 128.0); \
	GLfloat fz = (GLfloat) z * (1.0 / 128.0); \
	UPDATE_3D_BBOX(fx, fy, fz)

#define UPDATE_WITH_3D_FLOATS_UB_NORMALIZED() \
	GLfloat fx = (GLfloat) x * (1.0 / 255.0); \
	GLfloat fy = (GLfloat) y * (1.0 / 255.0); \
	GLfloat fz = (GLfloat) z * (1.0 / 255.0); \
	UPDATE_3D_BBOX(fx, fy, fz)

#define UPDATE_WITH_3D_FLOATS_US_NORMALIZED() \
	GLfloat fx = (GLfloat) x * (1.0 / 65535.0); \
	GLfloat fy = (GLfloat) y * (1.0 / 65535.0); \
	GLfloat fz = (GLfloat) z * (1.0 / 65535.0); \
	UPDATE_3D_BBOX(fx, fy, fz)


#define UPDATE_WITH_1D_VFLOATS() \
	UPDATE_1D_BBOX(v[0])

#define UPDATE_WITH_2D_VFLOATS() \
	UPDATE_2D_BBOX(v[0], v[1])

#define UPDATE_WITH_3D_VFLOATS() \
	UPDATE_3D_BBOX(v[0], v[1], v[2])

#define UPDATE_WITH_4D_VFLOATS() \
	GLfloat fx = (GLfloat) v[0]; \
	GLfloat fy = (GLfloat) v[1]; \
	GLfloat fz = (GLfloat) v[2]; \
	GLfloat fw = (GLfloat) v[3]; \
	fx /= fw; fy /= fw; fz/= fw; \
	UPDATE_3D_BBOX(fx, fy, fz)

/* For glVertexAttrib4N*ARB */
#define UPDATE_WITH_4D_FLOATS_UB_NORMALIZED() \
	GLfloat fx = (GLfloat) (x * (1.0 / 255.0)); \
	GLfloat fy = (GLfloat) (y * (1.0 / 255.0)); \
	GLfloat fz = (GLfloat) (z * (1.0 / 255.0)); \
	GLfloat fw = (GLfloat) (w * (1.0 / 255.0)); \
	fx /= fw; fy /= fw; fz/= fw; \
	UPDATE_3D_BBOX(fx, fy, fz)

#define UPDATE_WITH_4D_VFLOATS_B_NORMALIZED() \
	GLfloat fx = (GLfloat) (v[0] * (1.0 / 128.0)); \
	GLfloat fy = (GLfloat) (v[1] * (1.0 / 128.0)); \
	GLfloat fz = (GLfloat) (v[2] * (1.0 / 128.0)); \
	GLfloat fw = (GLfloat) (v[3] * (1.0 / 128.0)); \
	fx /= fw; fy /= fw; fz/= fw; \
	UPDATE_3D_BBOX(fx, fy, fz)

#define UPDATE_WITH_4D_VFLOATS_S_NORMALIZED() \
	GLfloat fx = (GLfloat) (2.0 * v[0] + 1.0) / ((GLfloat) (0xffff)); \
	GLfloat fy = (GLfloat) (2.0 * v[1] + 1.0) / ((GLfloat) (0xffff)); \
	GLfloat fz = (GLfloat) (2.0 * v[2] + 1.0) / ((GLfloat) (0xffff)); \
	GLfloat fw = (GLfloat) (2.0 * v[3] + 1.0) / ((GLfloat) (0xffff)); \
	fx /= fw; fy /= fw; fz/= fw; \
	UPDATE_3D_BBOX(fx, fy, fz)

#define UPDATE_WITH_4D_VFLOATS_I_NORMALIZED() \
	GLfloat fx = (GLfloat) (2.0 * v[0] + 1.0) / ((GLfloat) (0xffffffff)); \
	GLfloat fy = (GLfloat) (2.0 * v[1] + 1.0) / ((GLfloat) (0xffffffff)); \
	GLfloat fz = (GLfloat) (2.0 * v[2] + 1.0) / ((GLfloat) (0xffffffff)); \
	GLfloat fw = (GLfloat) (2.0 * v[3] + 1.0) / ((GLfloat) (0xffffffff)); \
	fx /= fw; fy /= fw; fz/= fw; \
	UPDATE_3D_BBOX(fx, fy, fz)

#define UPDATE_WITH_4D_VFLOATS_UB_NORMALIZED() \
	GLfloat fx = (GLfloat) (v[0] * (1.0 / 255.0)); \
	GLfloat fy = (GLfloat) (v[1] * (1.0 / 255.0)); \
	GLfloat fz = (GLfloat) (v[2] * (1.0 / 255.0)); \
	GLfloat fw = (GLfloat) (v[3] * (1.0 / 255.0)); \
	fx /= fw; fy /= fw; fz/= fw; \
	UPDATE_3D_BBOX(fx, fy, fz)

#define UPDATE_WITH_4D_VFLOATS_US_NORMALIZED() \
	GLfloat fx = (GLfloat) (v[0] * (1.0 / 65535.0)); \
	GLfloat fy = (GLfloat) (v[1] * (1.0 / 65535.0)); \
	GLfloat fz = (GLfloat) (v[2] * (1.0 / 65535.0)); \
	GLfloat fw = (GLfloat) (v[3] * (1.0 / 65535.0)); \
	fx /= fw; fy /= fw; fz/= fw; \
	UPDATE_3D_BBOX(fx, fy, fz)

#define UPDATE_WITH_4D_VFLOATS_UI_NORMALIZED() \
	GLfloat fx = v[0] / ((GLfloat) (0xffffffff)); \
	GLfloat fy = v[1] / ((GLfloat) (0xffffffff)); \
	GLfloat fz = v[2] / ((GLfloat) (0xffffffff)); \
	GLfloat fw = v[3] / ((GLfloat) (0xffffffff)); \
	fx /= fw; fy /= fw; fz/= fw; \
	UPDATE_3D_BBOX(fx, fy, fz)

#endif /* CR_PACK_BBOX_H */

