#include "cr_pack.h"
#include "cr_packfunctions.h"
#include "cr_mem.h"
#include "cr_timer.h"
#include "cr_error.h"

#define UNUSED( x ) ((void) (x))

void do_nothing( CROpcode opcode, void *buf )
{
	UNUSED( opcode );
	UNUSED( buf );
}

void reset_buffer( void *arg )
{
	crPackResetPointers( 0 );
	UNUSED(arg);
}

#define GRANULARITY 20000000

void MeasurePerformance( void )
{
	CRTimer *timer = crTimerNewTimer();
	int i;
	float vertices[3] = { 0,0,0 }; /* Whatever */
	double elapsed, start_time;
	int count = 0;

	crStartTimer( timer );
	start_time = crTimerTime( timer );

	for (;;)
	{
		for (i = 0; i < GRANULARITY ; i++)
		{
			/* Note: I don't like the word vertexes, but I'm willing 
			 * to live with it.  However, 'vertice' is NOT A WORD! */

			crPackVertex3fv(vertices);
		}
		count ++;
		elapsed = crTimerTime( timer ) - start_time;
		crDebug( "Pack Rate: %f vertices/sec", (count * GRANULARITY) / elapsed );
	}
}

int main( int argc, char *argv[] )
{
	CRPackBuffer pack_buffer;
	crPackInit( 0 ); /* Don't swap bytes, for God's sake */
	crPackInitBuffer( &pack_buffer, crAlloc( 1024*1024 ), 1024*1024, 0 );
	crPackSetBuffer( &pack_buffer );

	crPackFlushFunc( reset_buffer );
	crPackSendHugeFunc( do_nothing );

	MeasurePerformance();

	UNUSED( argc );
	UNUSED( argv );
	return 0;
}
