#ifndef TIMER_H
#define TIMER_H

typedef struct Timer {

	double time0;
	double elapsed;
	int    running;

} Timer;

void   TimerInit( Timer *timer );
double TimerGetTime( void );
void   TimerStart( Timer *timer );
void   TimerStop( Timer *timer );
void   TimerReset( Timer *timer );
double TimerTime( Timer *timer );

#endif /* TIMER_H */
