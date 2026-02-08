// timer.h
// Simple POSIX timer utility for Linux

#ifndef UTIL_TIMER_H
#define UTIL_TIMER_H

#include <time.h>
#include <signal.h>

typedef void (*timer_callback_t)(void *arg);

typedef struct {
    timer_t timerid;
    timer_callback_t callback;
    void *callback_arg;
    int signo;
} util_timer_t;

int util_timer_create(util_timer_t *t, timer_callback_t cb, void *arg, int signo);
int util_timer_start(util_timer_t *t, unsigned int delay_ms);
int util_timer_delete(util_timer_t *t);

#endif // UTIL_TIMER_H
