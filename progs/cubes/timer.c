/* timer.c */

#if defined( __sgi ) 
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/syssgi.h>
#include <sys/errno.h>
#include <unistd.h>
#elif defined( _WIN32 )
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#elif defined( Linux ) || defined( FreeBSD ) || defined(AIX) || defined(SunOS) || defined(OSF1)
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#else
#error TIMER ARCHITECTURE
#endif
#include "timer.h"

#if defined( __sgi )

typedef unsigned long long iotimer64_t;
typedef unsigned int iotimer32_t;

static struct {

	volatile iotimer64_t *addr64;
	volatile iotimer32_t *addr32;

	unsigned int cycleval;
	double       scale;

	int          initialized;
} __timer;

#elif defined( _WIN32 )

static struct {
	LARGE_INTEGER frequency;
	double        scale;

	int           initialized;
} __timer;

#elif defined( Linux ) || defined( FreeBSD ) || defined(AIX) || defined (SunOS) || defined(OSF1)

static struct {
	int initialized;
} __timer;

#endif

static void
__TimerInit( void )
{
#if defined( __sgi )
	__psunsigned_t phys_addr, raddr;
	int fd, poff_mask, counter_size;
	
	poff_mask = getpagesize() - 1;
	counter_size = syssgi(SGI_CYCLECNTR_SIZE);

	phys_addr = syssgi( SGI_QUERY_CYCLECNTR, &__timer.cycleval );
	if ( phys_addr == ENODEV ) 
	{
		fprintf( stderr, "Sorry, this SGI doesn't support timers.\n" );
		exit( 1 );
	}

	__timer.scale = 1e-12 * (double) __timer.cycleval;

	raddr = phys_addr & ~poff_mask;
	fd = open( "/dev/mmem", O_RDONLY );

	if ( counter_size == 64 ) 
	{
		__timer.addr64 = (iotimer64_t *)
			mmap( 0, poff_mask, PROT_READ, MAP_PRIVATE, fd, (off_t) raddr );
		__timer.addr64 = (iotimer64_t *)
			( (__psunsigned_t) __timer.addr64 + ( phys_addr & poff_mask ) );
	}
	else if ( counter_size == 32 ) 
	{
		__timer.addr32 = (iotimer32_t *)
			mmap( 0, poff_mask, PROT_READ, MAP_PRIVATE, fd, (off_t) raddr );
		__timer.addr32 = (iotimer32_t *)
			( (__psunsigned_t) __timer.addr32 + ( phys_addr & poff_mask ) );
	}
	else {
		fprintf( stderr, "Fatal timer init error\n" );
		exit( 1 );
	}
#elif defined( _WIN32 )
	QueryPerformanceFrequency( &__timer.frequency );
	__timer.scale = 1.0 / (double) __timer.frequency.QuadPart;
#endif
	__timer.initialized = 1;
}


void
TimerInit( Timer *timer )
{
	if ( !__timer.initialized )
		__TimerInit( );

	timer->time0   = 0;
	timer->elapsed = 0;
	timer->running = 0;
}


double 
TimerGetTime( void )
{
#if defined( __sgi )
	if ( !__timer.initialized )
		__TimerInit( );

	if ( __timer.addr64 ) {
		iotimer64_t counter_value = *__timer.addr64;
		return (double) counter_value * __timer.scale;
	}
	else {
		iotimer32_t counter_value = *__timer.addr32;
		return (double) counter_value * __timer.scale;
	}
#elif defined( _WIN32 )
	LARGE_INTEGER counter;

	if ( !__timer.initialized )
		__TimerInit( );

	QueryPerformanceCounter( &counter );
	return (double) counter.QuadPart * __timer.scale;
#elif defined( Linux ) || defined( FreeBSD ) || defined(AIX) || defined(SunOS) || defined(OSF1)
	struct timeval timeofday;

	if ( !__timer.initialized )
		__TimerInit( );

	if ( gettimeofday( &timeofday, NULL ) ) {
		perror( "gettimeofday" );
		exit( 1 );
	}
	return (double) timeofday.tv_sec + (double) timeofday.tv_usec * 1e-6;
#else
#error TIMER ARCH
#endif
}

void 
TimerStart( Timer *timer )
{
    timer->running = 1;
    timer->time0 = TimerGetTime( );
}

void 
TimerStop( Timer *timer )
{
    timer->running = 0;
    timer->elapsed += TimerGetTime( ) - timer->time0;
}

void 
TimerReset( Timer *timer )
{
    timer->running = 0;
    timer->elapsed = 0;
}

double 
TimerTime( Timer *timer )
{
	if ( timer->running ) {
		TimerStop( timer );
		TimerStart( timer );
	}
	return timer->elapsed;
}
