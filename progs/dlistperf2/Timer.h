#ifndef TIMER_H
#define TIMER_H

#include <sys/time.h>
#include <time.h>


extern struct timeval start_time;
extern struct timeval end_time;
extern clock_t cpu_time;


void Timer_Timer(void);
void Start(void);
double GetSeconds(void);
double GetCPUSeconds(void);
long GetMili(void);
#endif
