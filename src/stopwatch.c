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
  long long start_real_usec;
};
static struct Timing timing;

// Initializes PAPI library. Should only be called once.
// Will return 0 if successful
// Will return -1 if fail
int init_stopwatch() {
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

int start_stopwatch() {
  timing.start_real_cyc = PAPI_get_real_cyc();
  timing.start_real_usec = PAPI_get_real_usec();

  int PAPI_ret = PAPI_start(event_set);
  if (PAPI_ret != PAPI_OK) {
    return -1;
  }

  return 0;
}

int stop_stopwatch() {
  int PAPI_ret = PAPI_stop(event_set, results);
  if (PAPI_ret != PAPI_OK) {
    return -1;
  }

  return 0;
}

int destroy_stopwatch() {
  if (PAPI_cleanup_eventset(event_set) != PAPI_OK) {
    return -1;
  }

  if (PAPI_destroy_eventset(&event_set) != PAPI_OK) {
    return -1;
  }

  PAPI_shutdown();

  initialized_papi = 0;

  return 0;
}

int read_stopwatch(struct Measurements *reading) {
  int PAPI_ret = PAPI_read(event_set, results);
  if (PAPI_ret != PAPI_OK) {
    return -1;
  }
  reading->real_cyc_elapsed_since_start = PAPI_get_real_cyc() - timing.start_real_cyc;
  reading->real_usec_elapsed_since_start = PAPI_get_real_usec() - timing.start_real_usec;
  reading->l1_misses_since_start = results[0];
  reading->cyc_wait_resource_since_start = results[1];
  return 0;
}

void print_results(struct Measurements *start, struct Measurements *end) {
  printf("L1 cache misses: %lld\n", end->l1_misses_since_start - start->l1_misses_since_start);
  printf("Cycles waiting for resources: %lld\n",
         end->cyc_wait_resource_since_start - start->cyc_wait_resource_since_start);

  printf("Total real cycles elapsed: %lld\n", end->real_cyc_elapsed_since_start - start->real_cyc_elapsed_since_start);
  printf("Total real microseconds elapsed: %lld\n\n",
         end->real_usec_elapsed_since_start - start->real_usec_elapsed_since_start);
}
