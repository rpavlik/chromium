#ifndef CR_PACK_BBOX_H
#define CR_PACK_BBOX_H

#include "cr_pack.h"

#ifdef WINDOWS
#define UPDATE_2D_BBOX() \
__asm {\
	__asm fld fx\
	__asm fld cr_packer_globals.bounds_min.x\
	__asm fcomi st, st(1)\
	__asm fcmovnb st(0), st(1)\
	__asm fstp cr_packer_globals.bounds_min.x\
	__asm fld cr_packer_globals.bounds_max.x\
	__asm fcomi st, st(1)\
	__asm fcmovb st(0), st(1)\
	__asm fstp cr_packer_globals.bounds_max.x\
	__asm fstp fx\
	\
	__asm fld fy\
	__asm fld cr_packer_globals.bounds_min.y\
	__asm fcomi st, st(1)\
	__asm fcmovnb st(0), st(1)\
	__asm fstp cr_packer_globals.bounds_min.y\
	__asm fld cr_packer_globals.bounds_max.y\
	__asm fcomi st, st(1)\
	__asm fcmovb st(0), st(1)\
	__asm fstp cr_packer_globals.bounds_max.y\
	__asm fstp fy\
} \
	if (cr_packer_globals.bounds_min.z > 0.0f)  cr_packer_globals.bounds_min.z = 0.0f;\
	if (cr_packer_globals.bounds_max.z < 0.0f)  cr_packer_globals.bounds_max.z = 0.0f
#else
#define UPDATE_2D_BBOX() \
if (cr_packer_globals.bounds_min.x > fx)    cr_packer_globals.bounds_min.x = fx;\
if (cr_packer_globals.bounds_min.y > fy)    cr_packer_globals.bounds_min.y = fy;\
if (cr_packer_globals.bounds_min.z > 0.0f)  cr_packer_globals.bounds_min.z = 0.0f;\
if (cr_packer_globals.bounds_max.x < fx)    cr_packer_globals.bounds_max.x = fx;\
if (cr_packer_globals.bounds_max.y < fy)    cr_packer_globals.bounds_max.y = fy;\
if (cr_packer_globals.bounds_max.z < 0.0f)  cr_packer_globals.bounds_max.z = 0.0f
#endif

#ifdef WINDOWS
#define UPDATE_3D_BBOX() \
__asm {\
	__asm fld fx\
	__asm fld cr_packer_globals.bounds_min.x\
	__asm fcomi st, st(1)\
	__asm fcmovnb st(0), st(1)\
	__asm fstp cr_packer_globals.bounds_min.x\
	__asm fld cr_packer_globals.bounds_max.x\
	__asm fcomi st, st(1)\
	__asm fcmovb st(0), st(1)\
	__asm fstp cr_packer_globals.bounds_max.x\
	__asm fstp fx\
	\
	__asm fld fy\
	__asm fld cr_packer_globals.bounds_min.y\
	__asm fcomi st, st(1)\
	__asm fcmovnb st(0), st(1)\
	__asm fstp cr_packer_globals.bounds_min.y\
	__asm fld cr_packer_globals.bounds_max.y\
	__asm fcomi st, st(1)\
	__asm fcmovb st(0), st(1)\
	__asm fstp cr_packer_globals.bounds_max.y\
	__asm fstp fy\
	\
	__asm fld fz\
	__asm fld cr_packer_globals.bounds_min.z\
	__asm fcomi st, st(1)\
	__asm fcmovnb st(0), st(1)\
	__asm fstp cr_packer_globals.bounds_min.z\
	__asm fld cr_packer_globals.bounds_max.z\
	__asm fcomi st, st(1)\
	__asm fcmovb st(0), st(1)\
	__asm fstp cr_packer_globals.bounds_max.z\
	__asm fstp fz\
	\
}
#else
#define UPDATE_3D_BBOX() \
if (cr_packer_globals.bounds_min.x > fx)    cr_packer_globals.bounds_min.x = fx; \
if (cr_packer_globals.bounds_min.y > fy)    cr_packer_globals.bounds_min.y = fy; \
if (cr_packer_globals.bounds_min.z > fz)    cr_packer_globals.bounds_min.z = fz; \
if (cr_packer_globals.bounds_max.x < fx)    cr_packer_globals.bounds_max.x = fx; \
if (cr_packer_globals.bounds_max.y < fy)    cr_packer_globals.bounds_max.y = fy; \
if (cr_packer_globals.bounds_max.z < fz)    cr_packer_globals.bounds_max.z = fz
#endif

#ifdef SIMD
#define UPDATE_3D_BBOX_SIMD() \
__asm {\
	__asm movups xmm0, fx\
	__asm movaps xmm1, cr_packer_globals.bounds_min.x\
	__asm movaps xmm2, cr_packer_globals.bounds_max.x\
	__asm minps xmm1, xmm0\
	__asm maxps xmm2, xmm0\
	__asm movaps cr_packer_globals.bounds_min.x, xmm1\
	__asm movaps cr_packer_globals.bounds_max.x, xmm2\
}
#define UPDATE_3D_BBOX_SIMD_PACK() \
__asm {\
	__asm mov ecx, [data_ptr]\
	__asm movups xmm0, fx\
	__asm movaps xmm1, cr_packer_globals.bounds_min.x\
	__asm movaps xmm2, cr_packer_globals.bounds_max.x\
	__asm minps xmm1, xmm0\
	__asm maxps xmm2, xmm0\
	__asm movaps cr_packer_globals.bounds_min.x, xmm1\
	__asm movaps cr_packer_globals.bounds_max.x, xmm2\
	__asm movups [ecx], xmm0\
}
#define UPDATE_3DV_BBOX_SIMD() \
__asm {\
	__asm mov eax, [v]\
	__asm mov ecx, [data_ptr]\
	__asm movups xmm0, [eax]\
	__asm movaps xmm1, cr_packer_globals.bounds_min.x\
	__asm movaps xmm2, cr_packer_globals.bounds_max.x\
	__asm minps xmm1, xmm0\
	__asm maxps xmm2, xmm0\
	__asm movaps cr_packer_globals.bounds_min.x, xmm1\
	__asm movaps cr_packer_globals.bounds_max.x, xmm2\
	__asm movups [ecx], xmm0\
}
#else
#define UPDATE_3DV_BBOX_SIMD() { CREATE_3D_VFLOATS(); UPDATE_3D_BBOX();}
#define UPDATE_3D_BBOX_SIMD()  UPDATE_3D_BBOX()
#endif

#define CREATE_2D_FLOATS() \
	GLfloat fx = (GLfloat) x; \
	GLfloat fy = (GLfloat) y

#define CREATE_3D_FLOATS() \
	GLfloat fx = (GLfloat) x; \
	GLfloat fy = (GLfloat) y; \
	GLfloat fz = (GLfloat) z

#define CREATE_4D_FLOATS() \
	GLfloat fx = (GLfloat) x; \
	GLfloat fy = (GLfloat) y; \
	GLfloat fz = (GLfloat) z; \
	GLfloat fw = (GLfloat) w; \
	fx /= fw; fy /= fw; fz/= fw

#define CREATE_2D_VFLOATS() \
	GLfloat fx = (GLfloat) v[0]; \
	GLfloat fy = (GLfloat) v[1]

#define CREATE_3D_VFLOATS() \
	GLfloat fx = (GLfloat) v[0]; \
	GLfloat fy = (GLfloat) v[1]; \
	GLfloat fz = (GLfloat) v[2]

#define CREATE_4D_VFLOATS() \
	GLfloat fx = (GLfloat) v[0]; \
	GLfloat fy = (GLfloat) v[1]; \
	GLfloat fz = (GLfloat) v[2]; \
	GLfloat fw = (GLfloat) v[3]; \
	fx /= fw; fy /= fw; fz/= fw

#endif /* CR_PACK_BBOX_H */

