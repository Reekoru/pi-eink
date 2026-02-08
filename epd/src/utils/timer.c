#include "utils/timer.h"
#include <string.h>
#include <stdio.h>

static void timer_signal_handler(int signo, siginfo_t *si, void *unused) {
    util_timer_t *t = si->si_value.sival_ptr;
    if (t && t->callback) {
        t->callback(t->callback_arg);
    }
}

int util_timer_create(util_timer_t *t, timer_callback_t cb, void *arg, int signo) {
    struct sigevent sev;
    struct sigaction sa;
    memset(&sev, 0, sizeof(sev));
    memset(&sa, 0, sizeof(sa));

    t->callback = cb;
    t->callback_arg = arg;
    t->signo = signo;

    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = timer_signal_handler;
    sigemptyset(&sa.sa_mask);
    if (sigaction(signo, &sa, NULL) == -1) {
        perror("sigaction");
        return -1;
    }

    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = signo;
    sev.sigev_value.sival_ptr = t;
    if (timer_create(CLOCK_REALTIME, &sev, &t->timerid) == -1) {
        perror("timer_create");
        return -1;
    }
    return 0;
}

int util_timer_start(util_timer_t *t, unsigned int delay_ms) {
    struct itimerspec its;
    its.it_value.tv_sec = delay_ms / 1000;
    its.it_value.tv_nsec = (delay_ms % 1000) * 1000000;
    its.it_interval.tv_sec = 0;
    its.it_interval.tv_nsec = 0;
    if (timer_settime(t->timerid, 0, &its, NULL) == -1) {
        perror("timer_settime");
        return -1;
    }
    return 0;
}

int util_timer_delete(util_timer_t *t) {
    if (timer_delete(t->timerid) == -1) {
        perror("timer_delete");
        return -1;
    }
    return 0;
}
