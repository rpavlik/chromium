/*
 * Measure performance of the crPackVertex3fv function.
 */


#include "cr_pack.h"
#include "cr_packfunctions.h"
#include "cr_mem.h"
#include "cr_timer.h"
#include "cr_error.h"

#define UNUSED( x ) ((void) (x))

static CRPackContext *pack_context;

static void do_nothing( CROpcode opcode, void *buf )
{
	UNUSED( opcode );
	UNUSED( buf );
}

static void reset_buffer( void *arg )
{
	crPackResetPointers( pack_context );
	UNUSED(arg);
}

#define GRANULARITY 20000000

static void MeasurePerformance( void )
{
	CRTimer *timer = crTimerNewTimer();
	int i, j;
	float vertices[3] = { 0,0,0 }; /* Whatever */
	double elapsed, start_time;
	int count;

	crStartTimer( timer );

	count = 0;
	start_time = crTimerTime( timer );
	for (j = 0; j < 5; j++)
	{
		for (i = 0; i < GRANULARITY; i++)
		{
			/* Note: I don't like the word vertexes, but I'm willing 
			 * to live with it.  However, 'vertice' is NOT A WORD! */

			crPackVertex3fv(vertices);
		}
		count ++;
		elapsed = crTimerTime( timer ) - start_time;
		crDebug( "Pack Rate: %f glVertex3fv/sec", (count * GRANULARITY) / elapsed );
	}

	count = 0;
	start_time = crTimerTime( timer );
	for (j = 0; j < 5; j++)
	{
		for (i = 0; i < GRANULARITY; i++)
		{
			crPackVertex3f(vertices[0], vertices[1], vertices[2]);
		}
		count ++;
		elapsed = crTimerTime( timer ) - start_time;
		crDebug( "Pack Rate: %f glVertex3f/sec", (count * GRANULARITY) / elapsed );
	}
}

int main( int argc, char *argv[] )
{
	CRPackBuffer pack_buffer;

	crMemZero(&pack_buffer, sizeof(pack_buffer));

	pack_context = crPackNewContext( 0 ); /* Don't swap bytes, for God's sake */
	crPackSetContext( pack_context );
	crPackInitBuffer( &pack_buffer, crAlloc( 1024*1024 ), 1024*1024, 1024*1024 );
	crPackSetBuffer( pack_context, &pack_buffer );

	crPackFlushFunc( pack_context, reset_buffer );
	crPackSendHugeFunc( pack_context, do_nothing );

	MeasurePerformance();

	UNUSED( argc );
	UNUSED( argv );
	return 0;
}
