#include "Timer.h"

struct timeval start_time;
struct timeval end_time;
clock_t cpu_time;

void Timer_Timer(void)
{
    gettimeofday(&start_time, NULL);
    gettimeofday(&end_time, NULL);
}

void Start(void)
{
    gettimeofday(&start_time, NULL);
    cpu_time = clock();
}

long GetMili(void)
{
    long mili;

    gettimeofday(&end_time, NULL);

    mili = ((end_time.tv_sec - start_time.tv_sec) * 1000) +
	(end_time.tv_usec - start_time.tv_usec) / 1000;

    return (mili);
}

double GetSeconds(void)
{
    double secs;

    gettimeofday(&end_time, NULL);

    secs = (double) (end_time.tv_sec - start_time.tv_sec) +
	((double) (end_time.tv_usec - start_time.tv_usec) / 1000000.0);

    return (secs);
}

double GetCPUSeconds(void)
{
    clock_t tics = clock() - cpu_time;

    return ((double) tics / (double) CLOCKS_PER_SEC);

}
