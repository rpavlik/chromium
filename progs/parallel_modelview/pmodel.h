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

typedef struct {
	float x,y,z;
} Point;

typedef struct {
	Point min, max;
} BBOX;

typedef struct _mlist {
	int nverts,nfaces;
	Vertex *vlist;
	Face *flist;
	unsigned int *faces;
	BBOX bounds;
	struct _mlist *next;
} Model;

typedef struct {
	char path[8192];
	int num_tris;
	BBOX bounds;
} ModelFileInfo;

typedef struct {
	int node_id, num_nodes;

	char *ply_root;

	BBOX bounds;
	int total_triangles;
	Point center;
	float width, height,depth;
	float radius;

	Model *models;
	int num_models;

	int dpy_list_base;
	int window;
	int ctx;

	int has_nx,has_ny,has_nz;
	int has_r,has_g,has_b;
} Globals;

extern Globals globals;

void ReadFiles( void );

#endif /* PMODEL_H */
