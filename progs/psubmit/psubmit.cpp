#include <stdlib.h>
#include <iostream.h>
#include <mpi.h>
#include "cr_applications.h"

float verts[4][6] = {
	{ 0, 0, 1, 0, 1, 1 },
	{ 0, 0, -1, 0, -1, 1 },
	{ 0, 0, -1, 0, -1, -1 },
	{ 0, 0, 1, 0, 1, -1 }
};
float colors[7][3] = {
	{0,0,1},
	{0,1,0},
	{0,1,1},
	{1,0,0},
	{1,0,1},
	{1,1,0},
	{1,1,1}
};

static const int MASTER_BARRIER = 1;

int main(int argc, char *argv[])
{
	MPI_Init(&argc,&argv);

	int rank, size;
	MPI_Comm_rank( MPI_COMM_WORLD, &rank );
	MPI_Comm_size( MPI_COMM_WORLD, &size );

	cout << "I am " << rank << " of " << size << endl;

	crCreateContext();
	crMakeCurrent();

	if (rank == 0)
	{
		glBarrierCreate( MASTER_BARRIER, size );
	}

	MPI_Barrier( MPI_COMM_WORLD );

	for (;;)
	{
		if (rank == 0)
		{
			glClear( GL_COLOR_BUFFER_BIT );
		}

		glBarrierExec( MASTER_BARRIER );

		glRotatef(1,0,0,1);
		glBegin( GL_TRIANGLES );
		glColor3fv(colors[rank%7]);
		glVertex2f(verts[rank%4][0], verts[rank%4][1]);
		glVertex2f(verts[rank%4][2], verts[rank%4][3]);
		glVertex2f(verts[rank%4][4], verts[rank%4][5]);
		glEnd();

		glBarrierExec( MASTER_BARRIER );

		if (rank == 0)
		{
			crSwapBuffers();
		}

	}
	MPI_Finalize();
	return 0;
}
