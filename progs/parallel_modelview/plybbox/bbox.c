#include "pmodel.h"
#include "cr_mem.h"
#include "cr_error.h"

#include <float.h>

PlyProperty vert_props[] = { /* list of property information for a vertex */
	{"x", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,x), 0, 0, 0, 0},
	{"y", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,y), 0, 0, 0, 0},
	{"z", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,z), 0, 0, 0, 0},
};

int nverts;
Vertex *vlist;

static int nelems;
static char **elist;
static int file_type;

BBOX bounds;

int main( int argc, char *argv[] )
{
	int i,j;
	PlyFile *ply;
	int nprops;
	int num_elems;
	PlyProperty **plist;
	char *elem_name;
	float version;
	FILE *fp;

	if ((fp = fopen(argv[1], "rb")) == NULL)
	{
		crError( "Can't open %s for reading!", argv[1] );
	}

	bounds.maxx = bounds.maxy = bounds.maxz = -FLT_MAX;
	bounds.minx = bounds.miny = bounds.minz = FLT_MAX;

	ply  = ply_read (fp, &nelems, &elist);
	ply_get_info (ply, &version, &file_type);

	for (i = 0; i < nelems; i++) 
	{
		/* get the description of the first element */
		elem_name = elist[i];
		plist = ply_get_element_description (ply, elem_name, &num_elems, &nprops);

		if (equal_strings ("vertex", elem_name)) {

			/* create a vertex list to hold all the vertices */
			vlist = (Vertex *) crAlloc (sizeof (*vlist) * num_elems);
			nverts = num_elems;
			crDebug( "Reading %d vertices...", nverts );

			/* set up for getting vertex elements */

			ply_get_property (ply, elem_name, &vert_props[0]);
			ply_get_property (ply, elem_name, &vert_props[1]);
			ply_get_property (ply, elem_name, &vert_props[2]);

			/* grab all the vertex elements */
			for (j = 0; j < num_elems; j++) {
				ply_get_element (ply, vlist+j);
				if (vlist[j].x > bounds.maxx) bounds.maxx = vlist[j].x;
				if (vlist[j].y > bounds.maxy) bounds.maxy = vlist[j].y;
				if (vlist[j].z > bounds.maxz) bounds.maxz = vlist[j].z;
				if (vlist[j].x < bounds.minx) bounds.minx = vlist[j].x;
				if (vlist[j].y < bounds.miny) bounds.miny = vlist[j].y;
				if (vlist[j].z < bounds.minz) bounds.minz = vlist[j].z;
			}
		}
	}

	fprintf( stdout, "%f %f %f %f %f %f\n", bounds.minx, bounds.miny, bounds.minz, bounds.maxx, bounds.maxy, bounds.maxz );

	ply_close (ply);
	return 0;
}
