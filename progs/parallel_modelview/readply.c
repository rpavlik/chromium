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

int nverts,nfaces;
Vertex *vlist;
Face *flist;
unsigned int *faces;
int has_nx,has_ny,has_nz; /* are normals in PLY file? */
int has_r,has_g,has_b; /* are normals in PLY file? */

static PlyOtherElems *other_elements = NULL;
// static PlyOtherProp *vert_other,*face_other;
static int nelems;
static char **elist;
static int num_comments;
static char **comments;
static int num_obj_info;
static char **obj_info;
static int file_type;

void ReadFile( char *fname )
{
	int i,j;
	PlyFile *ply;
	int nprops;
	int num_elems;
	PlyProperty **plist;
	char *elem_name;
	float version;
	FILE *fp;

	/*** Read in the original PLY object ***/

	if ((fp = fopen(fname, "rb")) == NULL)
	{
		crError( "Can't open %s for reading!", fname );
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
			has_nx = has_ny = has_nz = 0;
			has_r = has_g = has_b = 0;
			for (j = 0; j < nprops; j++) {
				if (equal_strings ("nx", plist[j]->name)) has_nx = 1;
				if (equal_strings ("ny", plist[j]->name)) has_ny = 1;
				if (equal_strings ("nz", plist[j]->name)) has_nz = 1;
				if (equal_strings ("red", plist[j]->name)) has_r = 1;
				if (equal_strings ("green", plist[j]->name)) has_g = 1;
				if (equal_strings ("blue", plist[j]->name)) has_b = 1;
			}

			/* create a vertex list to hold all the vertices */
			vlist = (Vertex *) crAlloc (sizeof (*vlist) * num_elems);
			nverts = num_elems;
			crDebug( "Reading %d vertices...", nverts );

			/* set up for getting vertex elements */

			ply_get_property (ply, elem_name, &vert_props[0]);
			ply_get_property (ply, elem_name, &vert_props[1]);
			ply_get_property (ply, elem_name, &vert_props[2]);
			if (has_r) ply_get_property (ply, elem_name, &vert_props[3]);
			if (has_g) ply_get_property (ply, elem_name, &vert_props[4]);
			if (has_b) ply_get_property (ply, elem_name, &vert_props[5]);
			if (has_nx) ply_get_property (ply, elem_name, &vert_props[6]);
			if (has_ny) ply_get_property (ply, elem_name, &vert_props[7]);
			if (has_nz) ply_get_property (ply, elem_name, &vert_props[8]);
			// vert_other = ply_get_other_properties (ply, elem_name, devnull);

			/* grab all the vertex elements */
			for (j = 0; j < num_elems; j++) {
				vlist[j].r = 1.0f;
				vlist[j].g = 1.0f;
				vlist[j].b = 1.0f;
				vlist[j].nx = 1.0f;
				vlist[j].ny = 1.0f;
				vlist[j].nz = 1.0f;
				ply_get_element (ply, vlist+j);
				vlist[j].r /= 255.0f;
				vlist[j].g /= 255.0f;
				vlist[j].b /= 255.0f;
			}
		}
		else if (equal_strings ("face", elem_name)) {

			/* create a list to hold all the face elements */
			flist = (Face *) crAlloc (sizeof (*flist) * num_elems);
			nfaces = num_elems;

			/* set up for getting face elements */

			ply_get_property (ply, elem_name, &face_props[0]);
			// face_other = ply_get_other_properties (ply, elem_name,devnull);

			/* grab all the face elements */
			crDebug( "Reading %d faces...", nfaces );
			faces = (unsigned int *) crAlloc( nfaces * 3 * sizeof( *faces ) );
			for (j = 0; j < num_elems; j++) {
				ply_get_element (ply, flist + j);
				faces[j*3+0] = flist[j].verts[0];
				faces[j*3+1] = flist[j].verts[1];
				faces[j*3+2] = flist[j].verts[2];
			}
		}
		else
			other_elements = ply_get_other_element (ply, elem_name, num_elems);
	}

	comments = ply_get_comments (ply, &num_comments);
	obj_info = ply_get_obj_info (ply, &num_obj_info);

	ply_close (ply);
}
