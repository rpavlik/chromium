#include <float.h>

#include "pmodel.h"
#include "cr_mem.h"
#include "cr_error.h"

PlyProperty vert_props[] = { /* list of property information for a vertex */
	{"x", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,x), 0, 0, 0, 0},
	{"y", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,y), 0, 0, 0, 0},
	{"z", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,z), 0, 0, 0, 0},
	{"red", PLY_UCHAR, PLY_FLOAT, offsetof(Vertex,r), 0, 0, 0, 0},
	{"green", PLY_UCHAR, PLY_FLOAT, offsetof(Vertex,g), 0, 0, 0, 0},
	{"blue", PLY_UCHAR, PLY_FLOAT, offsetof(Vertex,b), 0, 0, 0, 0},
	{"nx", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,nx), 0, 0, 0, 0},
	{"ny", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,ny), 0, 0, 0, 0},
	{"nz", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,nz), 0, 0, 0, 0},
};

PlyProperty face_props[] = { /* list of property information for a vertex */
	{"vertex_indices", PLY_INT, PLY_INT, offsetof(Face,verts),
		1, PLY_UCHAR, PLY_UCHAR, offsetof(Face,nverts)},
};

static int nelems;
static char **elist;
static int file_type;

typedef struct mfi_list {
	ModelFileInfo file_info;
	struct mfi_list *next;
} MFIList;

MFIList *mfi_head = NULL, *mfi_tail = NULL;

void ParseModelInfo( void )
{
	FILE *fp;
	char fname[1024];
	sprintf( fname, "%s/model_info", globals.ply_root );
	if ((fp = fopen( fname, "r" )) == NULL)
	{
		crError( "Can't open %s", fname );
	}

	globals.total_triangles = 0;
	globals.global_bounds.max.x = globals.global_bounds.max.y = globals.global_bounds.max.z = -FLT_MAX;
	globals.global_bounds.min.x = globals.global_bounds.min.y = globals.global_bounds.min.z = FLT_MAX;

	for (;;)
	{
		MFIList *mfi;
		char buf[8192];
		char path[8192];
		fgets( buf, 8192, fp );
		if (feof(fp))
		{
			break;
		}
		mfi = (MFIList *) crAlloc( sizeof( *mfi ) );
		sscanf( buf, "%s %d %f %f %f %f %f %f", 
				path, &(mfi->file_info.num_tris),
				&(mfi->file_info.bounds.min.x),
				&(mfi->file_info.bounds.min.y),
				&(mfi->file_info.bounds.min.z),
				&(mfi->file_info.bounds.max.x),
				&(mfi->file_info.bounds.max.y),
				&(mfi->file_info.bounds.max.z) );
		sprintf( mfi->file_info.path, "%s/%s", globals.ply_root, path );
#if 1
		mfi->next = mfi_head;
		mfi_head = mfi;
#else
		mfi->next = NULL;
		if (mfi_tail != NULL) mfi_tail->next = mfi;
		else mfi_head = mfi;
		mfi_tail = mfi;
#endif
		globals.total_triangles += mfi->file_info.num_tris;
		if (mfi->file_info.bounds.min.x < globals.global_bounds.min.x) {
			globals.global_bounds.min.x = mfi->file_info.bounds.min.x;
		}
		if (mfi->file_info.bounds.min.y < globals.global_bounds.min.y) {
			globals.global_bounds.min.y = mfi->file_info.bounds.min.y;
		}
		if (mfi->file_info.bounds.min.z < globals.global_bounds.min.z) {
			globals.global_bounds.min.z = mfi->file_info.bounds.min.z;
		}
		if (mfi->file_info.bounds.max.x > globals.global_bounds.max.x) {
			globals.global_bounds.max.x = mfi->file_info.bounds.max.x;
		}
		if (mfi->file_info.bounds.max.y > globals.global_bounds.max.y) {
			globals.global_bounds.max.y = mfi->file_info.bounds.max.y;
		}
		if (mfi->file_info.bounds.max.z > globals.global_bounds.max.z) {
			globals.global_bounds.max.z = mfi->file_info.bounds.max.z;
		}
	}
	crDebug( "I'm done with the model info file" );
	fclose( fp );
}

void ReadFile( MFIList *mfi, Model *model )
{
	int i,j;
	PlyFile *ply;
	int nprops;
	int num_elems;
	PlyProperty **plist;
	char *elem_name;
	float version;
	FILE *fp;

	if ((fp = fopen(mfi->file_info.path, "rb")) == NULL)
	{
		crError( "Can't open %s for reading!", mfi->file_info.path );
	}

	ply  = ply_read (fp, &nelems, &elist);
	ply_get_info (ply, &version, &file_type);

	for (i = 0; i < nelems; i++) 
	{
		/* get the description of the first element */
		elem_name = elist[i];
		plist = ply_get_element_description (ply, elem_name, &num_elems, &nprops);

		if (equal_strings ("vertex", elem_name)) {

			/* see if vertex holds any normal information */
			globals.has_nx = globals.has_ny = globals.has_nz = 0;
			globals.has_r = globals.has_g = globals.has_b = 0;
			for (j = 0; j < nprops; j++) {
				if (equal_strings ("nx", plist[j]->name)) globals.has_nx = 1;
				if (equal_strings ("ny", plist[j]->name)) globals.has_ny = 1;
				if (equal_strings ("nz", plist[j]->name)) globals.has_nz = 1;
				if (equal_strings ("red", plist[j]->name)) globals.has_r = 1;
				if (equal_strings ("green", plist[j]->name)) globals.has_g = 1;
				if (equal_strings ("blue", plist[j]->name)) globals.has_b = 1;
			}

			/* create a vertex list to hold all the vertices */
			model->vlist = (Vertex *) crAlloc (sizeof (*(model->vlist)) * num_elems);
			model->nverts = num_elems;

			/* set up for getting vertex elements */

			ply_get_property (ply, elem_name, &vert_props[0]);
			ply_get_property (ply, elem_name, &vert_props[1]);
			ply_get_property (ply, elem_name, &vert_props[2]);
			if (globals.has_r) ply_get_property (ply, elem_name, &vert_props[3]);
			if (globals.has_g) ply_get_property (ply, elem_name, &vert_props[4]);
			if (globals.has_b) ply_get_property (ply, elem_name, &vert_props[5]);
			if (globals.has_nx) ply_get_property (ply, elem_name, &vert_props[6]);
			if (globals.has_ny) ply_get_property (ply, elem_name, &vert_props[7]);
			if (globals.has_nz) ply_get_property (ply, elem_name, &vert_props[8]);

			/* grab all the vertex elements */
			for (j = 0; j < num_elems; j++) {
				model->vlist[j].r = 1.0f;
				model->vlist[j].g = 1.0f;
				model->vlist[j].b = 1.0f;
				model->vlist[j].nx = 1.0f;
				model->vlist[j].ny = 1.0f;
				model->vlist[j].nz = 1.0f;
				ply_get_element (ply, model->vlist+j);
				model->vlist[j].r /= 255.0f;
				model->vlist[j].g /= 255.0f;
				model->vlist[j].b /= 255.0f;
			}
		}
		else if (equal_strings ("face", elem_name)) {

			/* create a list to hold all the face elements */
			model->flist = (Face *) crAlloc (sizeof (*(model->flist)) * num_elems);
			model->nfaces = num_elems;

			/* set up for getting face elements */

			ply_get_property (ply, elem_name, &face_props[0]);

			/* grab all the face elements */
			crDebug( "Reading %s (%d faces)", mfi->file_info.path, model->nfaces );
			model->faces = (unsigned int *) crAlloc( model->nfaces * 3 * sizeof( *(model->faces) ) );
			for (j = 0; j < num_elems; j++) {
				ply_get_element (ply, model->flist + j);
				model->faces[j*3+0] = model->flist[j].verts[0];
				model->faces[j*3+1] = model->flist[j].verts[1];
				model->faces[j*3+2] = model->flist[j].verts[2];
			}
		}
	}

	ply_close (ply);
}
void ReadFiles( void )
{
	int node_id = 0;
	int cur_node_tris;
	MFIList *mfi;
	int tris_per_node;

	ParseModelInfo();
	crDebug( "Parsed the model information!" );
	crDebug( "Total triangles: %d", globals.total_triangles );
	crDebug( "Number of nodes: %d", globals.num_nodes );

	tris_per_node = globals.total_triangles / globals.num_nodes;
	globals.models = NULL;
	globals.num_models = 0;

	tris_per_node = 500000;

	crDebug( "Triangles per node: %d", tris_per_node );

	globals.local_bounds.max.x = globals.local_bounds.max.y = globals.local_bounds.max.z = -FLT_MAX;
	globals.local_bounds.min.x = globals.local_bounds.min.y = globals.local_bounds.min.z = FLT_MAX;


	for (mfi = mfi_head ; mfi ; mfi = mfi->next)
	{
		if (node_id == globals.node_id)
		{
			Model *model = (Model *) crAlloc( sizeof( *model ) );
			ReadFile( mfi, model );
			if (mfi->file_info.bounds.min.x < globals.local_bounds.min.x) {
				globals.local_bounds.min.x = mfi->file_info.bounds.min.x;
			}
			if (mfi->file_info.bounds.min.y < globals.local_bounds.min.y) {
				globals.local_bounds.min.y = mfi->file_info.bounds.min.y;
			}
			if (mfi->file_info.bounds.min.z < globals.local_bounds.min.z) {
				globals.local_bounds.min.z = mfi->file_info.bounds.min.z;
			}
			if (mfi->file_info.bounds.max.x > globals.local_bounds.max.x) {
				globals.local_bounds.max.x = mfi->file_info.bounds.max.x;
			}
			if (mfi->file_info.bounds.max.y > globals.local_bounds.max.y) {
				globals.local_bounds.max.y = mfi->file_info.bounds.max.y;
			}
			if (mfi->file_info.bounds.max.z > globals.local_bounds.max.z) {
				globals.local_bounds.max.z = mfi->file_info.bounds.max.z;
			}
			model->bounds = mfi->file_info.bounds;
			model->next = globals.models;
			globals.models = model;
			globals.num_models++;
		}
		cur_node_tris += mfi->file_info.num_tris;
		if (cur_node_tris >= tris_per_node)
		{
			node_id++;
			cur_node_tris = 0;
		}
		if (node_id > globals.node_id)
		{
			break;
		}
	}
}

