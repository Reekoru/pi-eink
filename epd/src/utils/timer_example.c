// Example usage for util_timer
#include "utils/timer.h"
#include <stdio.h>

void my_callback(void *arg) {
    printf("Timer expired! arg=%p\n", arg);
}

int main() {
    util_timer_t t;
    util_timer_create(&t, my_callback, NULL, SIGRTMIN);
    util_timer_start(&t, 2000); // 2 seconds
    printf("Timer started. Waiting...\n");
    // Sleep to wait for timer
    sleep(3);
    util_timer_delete(&t);
    return 0;
}
