#ifndef PMODEL_H
#define PMODEL_H

#include "ply.h"

typedef struct {
	// order matters, since I'm using interleaved arrays.
	float r,g,b;
	float nx, ny, nz;
	float x,y,z;
} Vertex;

typedef struct {
	unsigned char nverts;
	int *verts;
} Face;

extern int nverts,nfaces;
extern Vertex *vlist;
extern Face *flist;
extern unsigned int *faces;
extern int has_nx,has_ny,has_nz;
extern int has_r,has_g,has_b;

typedef struct {
	float maxx, maxy, maxz, minx, miny, minz;
} BBOX;

extern BBOX bounds;

void ReadFile( char *fname );

#endif /* PMODEL_H */
