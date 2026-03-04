#ifndef METRICS_H
#define METRICS_H

#include <stdint.h>

/*
 * metrics.h — Per-decision performance metrics for AI search.
 *
 * Every AI mode populates a Metrics struct during its search.
 * The benchmark harness reads these after each decision.
 */

typedef struct {
    uint64_t nodes_visited;     /* search nodes expanded */
    int      depth_requested;   /* depth the caller asked for */
    int      depth_completed;   /* depth actually completed (== requested for fixed) */
    double   time_ms;           /* wall-clock milliseconds for the decision */
    uint64_t tt_probes;         /* transposition table lookups (Mode 3+) */
    uint64_t tt_hits;           /* transposition table cache hits (Mode 3+) */

    /* Internal: timer start point (opaque, platform-specific). */
    double _start_ms;
} Metrics;

/* Reset all fields to zero and store depth_requested. */
void metrics_reset(Metrics *m, int depth_requested);

/* Record the start time (call just before search begins). */
void metrics_start_timer(Metrics *m);

/* Record the end time and compute time_ms (call just after search ends). */
void metrics_stop_timer(Metrics *m);

#endif /* METRICS_H */
