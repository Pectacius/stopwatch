#include <stdio.h>

#include "stopwatch/stopwatch.h"
#include "papi.h"

#define NUM_OF_EVENTS 2

static int initialized_papi = 0;

static int events[NUM_OF_EVENTS] = {PAPI_L1_TCM, PAPI_RES_STL};
static int event_set = PAPI_NULL;

static long long results[NUM_OF_EVENTS] = {0, 0};
struct Timing {
    long long start_real_cyc;
    long long start_virt_cyc;
    long long start_real_usec;
    long long start_virt_usec;
    long long end_real_cyc;
    long long end_virt_cyc;
    long long end_real_usec;
    long long end_virt_usec;
};
static struct Timing timing;


// Initializes PAPI library. Should only be called once.
// Will return 0 if successful
// Will return -1 if fail
int init_stopwatch()
{
    // Check if PAPI is not already initialized
    if (!initialized_papi) {
        int PAPI_ret = PAPI_library_init(PAPI_VER_CURRENT);
        if (PAPI_ret != PAPI_VER_CURRENT) {
            return -1;
        }

        PAPI_ret = PAPI_create_eventset(&event_set);
        if (PAPI_ret != PAPI_OK) {
            return -1;
        }

        PAPI_ret = PAPI_add_events(event_set, events, NUM_OF_EVENTS);
        if (PAPI_ret != PAPI_OK) {
            return -1;
        }

        initialized_papi = 1;
        return 0;
    }
    return -1;
}

int start_measurement()
{
    timing.start_real_cyc = PAPI_get_real_cyc();
    timing.start_real_usec = PAPI_get_real_usec();
    timing.start_virt_cyc = PAPI_get_virt_cyc();
    timing.start_virt_usec = PAPI_get_virt_usec();

    int PAPI_ret = PAPI_start(event_set);
    if (PAPI_ret != PAPI_OK) {
        return -1;
    }

    return 0;
}

int stop_measurement(struct Measurements* result)
{
    int PAPI_ret = PAPI_stop(event_set, results);
    if (PAPI_ret != PAPI_OK) {
        return -1;
    }

    timing.end_real_cyc = PAPI_get_real_cyc();
    timing.end_real_usec = PAPI_get_real_usec();
    timing.end_virt_cyc = PAPI_get_virt_cyc();
    timing.end_virt_usec = PAPI_get_virt_usec();

    result->l1_misses = results[0];
    result->cyc_wait_resource = results[1];
    result->real_cyc_elapsed = timing.end_real_cyc - timing.start_real_cyc;
    result->real_usec_elapsed = timing.end_real_usec - timing.start_real_usec;
    result->virt_cyc_elapsed = timing.end_virt_cyc - timing.start_virt_cyc;
    result->virt_usec_elapsed = timing.end_virt_usec - timing.start_virt_usec;

    return 0;
}

void print_results(struct Measurements* result)
{
    printf("L1 cache misses: %lld\n", result->l1_misses);
    printf("Cycles waiting for resources: %lld\n", result->cyc_wait_resource);

    printf("Total real cycles elapsed: %lld\n", result->real_cyc_elapsed);
    printf("Total virtual cycles elapsed: %lld\n", result->virt_cyc_elapsed);
    printf("Total real microseconds elapsed: %lld\n", result->real_usec_elapsed);
    printf("Total virtual microseconds elapsed: %lld\n\n", result->virt_usec_elapsed);
}
