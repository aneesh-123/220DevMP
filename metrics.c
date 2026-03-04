#include "metrics.h"
#include <string.h>
#include <time.h>

/* Cross-platform wall-clock time in milliseconds. */
static double now_ms(void) {
#if defined(_WIN32) || defined(_WIN64)
    /* clock() on Windows MSVC measures wall time;
     * on MinGW it's CPU time — acceptable approximation for POC. */
    return (double)clock() / CLOCKS_PER_SEC * 1000.0;
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1.0e6;
#endif
}

void metrics_reset(Metrics *m, int depth_requested) {
    memset(m, 0, sizeof(*m));
    m->depth_requested = depth_requested;
}

void metrics_start_timer(Metrics *m) {
    m->_start_ms = now_ms();
}

void metrics_stop_timer(Metrics *m) {
    m->time_ms = now_ms() - m->_start_ms;
}
