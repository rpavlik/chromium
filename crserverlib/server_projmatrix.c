/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "server_dispatch.h"
#include "server.h"
#include "cr_error.h"
#include "state/cr_statetypes.h"

void SERVER_DISPATCH_APIENTRY crServerDispatchLoadMatrixf( const GLfloat *m )
{
	const CRMuralInfo *mural = cr_server.curClient->currentMural;

	crStateLoadMatrixf( m );
	if (mural->numExtents > 0 && cr_server.curClient->currentCtx->transform.mode == GL_PROJECTION)
	{
		/* we're loading a matrix onto the projection stack -- better put the base 
		 * projection there first! */

		crServerApplyBaseProjection();
	}
	else
	{
		cr_server.head_spu->dispatch_table.LoadMatrixf( m );
	}
}

void SERVER_DISPATCH_APIENTRY crServerDispatchLoadMatrixd( const GLdouble *m )
{
	const CRMuralInfo *mural = cr_server.curClient->currentMural;

	crStateLoadMatrixd( m );
	if (mural->numExtents > 0 && cr_server.curClient->currentCtx->transform.mode == GL_PROJECTION)
	{
		/* we're loading a matrix onto the projection stack -- better put the base 
		 * projection there first! */

		crServerApplyBaseProjection();
	}
	else
	{
		cr_server.head_spu->dispatch_table.LoadMatrixd( m );
	}
}

void SERVER_DISPATCH_APIENTRY crServerDispatchLoadIdentity( void )
{
  const CRMuralInfo *mural = cr_server.curClient->currentMural;

	crStateLoadIdentity();
	if (mural->numExtents > 0 && cr_server.curClient->currentCtx->transform.mode == GL_PROJECTION)
	{
		/* we're loading a matrix onto the projection stack -- better put the base 
		 * projection there first! */

		crServerApplyBaseProjection();
	}
	else
	{
		cr_server.head_spu->dispatch_table.LoadIdentity( );
	}
}
