#include <stdio.h>

#include "stopwatch/stopwatch.h"
#include "papi.h"

#define NUM_OF_EVENTS 2

static int initialized_stopwatch = 0;

static int events[NUM_OF_EVENTS] = {PAPI_L1_TCM, PAPI_RES_STL};
static int event_set = PAPI_NULL;

static long long results[NUM_OF_EVENTS] = {0, 0};

static int start_stopwatch() {
  int PAPI_ret = PAPI_start(event_set);
  if (PAPI_ret != PAPI_OK) {
    return -1;
  }
  return 0;
}

// Initializes PAPI library. Should only be called once.
// Will return 0 if successful
// Will return -1 if fail
int init_event_timers() {
  // Check if stopwatch is not already initialized
  if (!initialized_stopwatch) {
    int ret_val = PAPI_library_init(PAPI_VER_CURRENT);
    if (ret_val != PAPI_VER_CURRENT) {
      return STOPWATCH_ERR;
    }

    ret_val = PAPI_create_eventset(&event_set);
    if (ret_val != PAPI_OK) {
      return STOPWATCH_ERR;
    }

    ret_val = PAPI_add_events(event_set, events, NUM_OF_EVENTS);
    if (ret_val != PAPI_OK) {
      return STOPWATCH_ERR;
    }

    // Start the monotonic clock
    ret_val = start_stopwatch();

    initialized_stopwatch = 1;

    // Since start_stopwatch is the last function being executed, its success or error can then be propagated to the
    // caller of init_stopwatch. If start_stopwatch errors out, then init_stopwatch has not been successfully executed.
    // If start_stopwatch is successful, and since there are no other functions being called, start_stopwatch's success
    // is the same as init_stopwatch's success.
    return ret_val;
  }
  return -1;
}

static int stop_stopwatch() {
  int PAPI_ret = PAPI_stop(event_set, results);
  if (PAPI_ret != PAPI_OK) {
    return -1;
  }
  return 0;
}

int destroy_event_timers() {
  int ret_val = stop_stopwatch();
  if (ret_val != 0) {
    return STOPWATCH_ERR;
  }

  ret_val = PAPI_cleanup_eventset(event_set);
  if (ret_val != PAPI_OK) {
    return STOPWATCH_ERR;
  }
  ret_val = PAPI_destroy_eventset(&event_set);
  if (ret_val != PAPI_OK) {
    return STOPWATCH_ERR;
  }

  PAPI_shutdown();

  initialized_stopwatch = 0;

  return STOPWATCH_OK;
}

struct StopwatchReadings create_stopwatch_readings() {
  struct StopwatchReadings default_val = {0,0,0,0, 0, 0,0,0,0};
  return default_val;
}

int record_start_measurements(struct StopwatchReadings* readings) {
  int PAPI_ret = PAPI_read(event_set, results);
  if (PAPI_ret != PAPI_OK) {
    return STOPWATCH_ERR;
  }
  readings->start_real_cyc = PAPI_get_real_cyc();
  readings->start_real_usec = PAPI_get_real_usec();
  readings->start_l1_misses = results[0];
  readings->start_cyc_wait_resource = results[1];

  return STOPWATCH_OK;
}

int record_end_measurements(struct StopwatchReadings* readings) {
  int PAPI_ret = PAPI_read(event_set, results);
  if (PAPI_ret != PAPI_OK) {
    return STOPWATCH_ERR;
  }

  readings->total_times_called++;

  readings->total_real_cyc += (PAPI_get_real_cyc() - readings->start_real_cyc);
  readings->total_real_usec +=  (PAPI_get_real_usec() - readings->start_real_usec);
  readings->total_l1_misses += (results[0] - readings->start_l1_misses);
  readings->total_cyc_wait_resource += (results[1] - readings->start_cyc_wait_resource);

  return STOPWATCH_OK;
}

void print_results(struct StopwatchReadings *readings) {
  printf("Total times run: %lld\n", readings->total_times_called);
  printf("L1 cache misses: %lld\n", readings->total_l1_misses);
  printf("Cycles waiting for resources: %lld\n", readings->total_cyc_wait_resource);

  printf("Total real cycles elapsed: %lld\n", readings->total_real_cyc);
  printf("Total real microseconds elapsed: %lld\n\n", readings->total_real_usec);
}
