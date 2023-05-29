#define _POSIX_C_SOURCE 199309L
#include "timer.h"

void
timer_start(Timer * timer)
{
    clock_gettime(CLOCK_MONOTONIC, &timer->start_time);
}

void
timer_stop(Timer * timer)
{
    clock_gettime(CLOCK_MONOTONIC, &timer->end_time);
}

long
timer_get_elapsed(Timer * timer, int unit)
{
    time_t dsec  = timer->end_time.tv_sec  - timer->start_time.tv_sec;
    long   dnsec = timer->end_time.tv_nsec - timer->start_time.tv_nsec;
    long result = 0;
    switch (unit) {
        case TIMER_S:  result = (dsec * 1000000000 + dnsec)/1000000000; break;
        case TIMER_MS: result = (dsec * 1000000000 + dnsec)/1000000;    break;
        case TIMER_US: result = (dsec * 1000000000 + dnsec)/1000;       break;
        case TIMER_NS: result = (dsec * 1000000000 + dnsec)/1;          break;
    }
    return result;
}
