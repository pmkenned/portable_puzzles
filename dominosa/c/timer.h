#ifndef TIMER_H
#define TIMER_H

#if !defined(_POSIX_C_SOURCE) || _POSIX_C_SOURCE < 199309L
#error "must define _POSIX_C_SOURCE >= 199309L"
#endif

#include <time.h>

typedef struct {
    struct timespec start_time, end_time;
} Timer;

enum { TIMER_NS, TIMER_US, TIMER_MS, TIMER_S };

void timer_start(Timer * timer);
void timer_stop(Timer * timer);
long timer_get_elapsed(Timer * timer, int unit);

#endif /* TIMER_H */
