#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "stopwatch/stopwatch.h"
#include "math_fun_util.h"
#include "assertion_util.h"

// Does a simple performance measurement on a matrix multiplication
void test_stopwatch_perf_mat_mul() {
  int N = 1000; // Square matrix size
  struct StopwatchMeasurementResult result;

  float (*A)[N] = calloc(sizeof(float), N * N);
  float (*B)[N] = calloc(sizeof(float), N * N);
  float (*C)[N] = calloc(sizeof(float), N * N);

  for (int jj = 0; jj < N; jj++) {
    for (int ii = 0; ii < N; ii++) {
      A[ii][jj] = ii + jj;
      B[ii][jj] = ii * jj;
    }
  }
  memset(C, 0, sizeof(float) * N * N);

  const enum StopwatchEvents events[] = {L1_CACHE_MISS, CYCLES_STALLED_RESOURCE};
  const unsigned int num_events = 2;
  assert(stopwatch_init(events, num_events) == STOPWATCH_OK);
  assert(stopwatch_record_start_measurements(1, "mat-mul", 0) == STOPWATCH_OK);
  row_major(N, A, B, C);
  assert(stopwatch_record_end_measurements(1) == STOPWATCH_OK);

  // Check the function name and stack depth are correct
  assert(stopwatch_get_measurement_results(1, &result) == STOPWATCH_OK);
  assert(result.caller_routine_id == 0);
  assert(strcmp(result.routine_name, "mat-mul") == 0);

  // check results. At the moment there is not exactly a precise way to assert that the produced value is accurate due
  // to various factors such as hardware specs and randomness from the OS. Hence these assertions at the moment will
  // only check that the values are not absurd i.e the total time is not 0 etc, until a method is devised that can
  // accurately check these values while also being hardware independent.
  assert(result.total_real_cyc > 0);
  assert(result.total_real_usec > 0);
  assert(result.total_event_values[0] > 0); // L1 cache misses
  assert(result.total_event_values[1] > 0); // Total cycles stalled waiting on resources

  assert(result.total_times_called == 1);

  // the total amount of cpu cycles should always be greater than the cycles waiting for resources, otherwise the cpu
  // does no work which cannot be as floating point operations are being performed.
  assert(result.total_real_cyc > result.total_event_values[1]);

  stopwatch_destroy();
}

// Repeatedly does the same matrix multiplication to test the accumulation feature
void test_stopwatch_perf_mat_mul_loop() {
  const enum StopwatchEvents events[] = {CYCLES_STALLED_RESOURCE, L1_CACHE_MISS};
  assert(stopwatch_init(events, 2) == STOPWATCH_OK);

  int N = 1000;
  int itercount = 10;

  // Structure for results
  struct StopwatchMeasurementResult result;

  // Allocate the A, B, and C arrays on the heap.
  // See https://stackoverflow.com/questions/10116368/heap-allocate-a-2d-array-not-array-of-pointers
  // for the syntax; C is not exactly the nicest language for N-dimension arrays
  float (*A)[N], (*B)[N], (*C)[N];

  // Use calloc for the allocation to initialize the memory to 0
  A = calloc(sizeof(float), N * N);
  B = calloc(sizeof(float), N * N);
  C = calloc(sizeof(float), N * N);

  // Initialize A and B to token values: i+j and i*j.  Memory accesses are base-zero.
  for (int ii = 0; ii < N; ii++) {
    for (int jj = 0; jj < N; jj++) {
      A[ii][jj] = ii + jj;
      B[ii][jj] = ii * jj;
    }
  }

  assert(stopwatch_record_start_measurements(1, "total-loop", 0) == STOPWATCH_OK);

  for (int iter = 0; iter < itercount; iter++) {
    // clear C array
    memset(C, 0, sizeof(float) * N * N);

    // read start time
    assert(stopwatch_record_start_measurements(2, "single-cycle", 1) == STOPWATCH_OK);
    // Perform the multiplication
    row_major(N, A, B, C);

    // read end time
    assert(stopwatch_record_end_measurements(2) == STOPWATCH_OK);
  }

  assert(stopwatch_record_end_measurements(1) == STOPWATCH_OK);

  free(A);
  free(B);
  free(C);

  struct StopwatchMeasurementResult total_loop;
  struct StopwatchMeasurementResult single_cycle;

  assert(stopwatch_get_measurement_results(1, &total_loop) == STOPWATCH_OK);
  assert(total_loop.caller_routine_id == 0);
  assert(strcmp(total_loop.routine_name, "total-loop") == 0);

  assert(stopwatch_get_measurement_results(2, &single_cycle) == STOPWATCH_OK);
  assert(single_cycle.caller_routine_id == 1);
  assert(strcmp(single_cycle.routine_name, "single-cycle") == 0);

  assert(total_loop.total_real_cyc > 0);
  assert(total_loop.total_real_usec > 0);
  assert(total_loop.total_event_values[0] > 0);
  assert(total_loop.total_event_values[1] > 0);
  assert(total_loop.total_times_called == 1);
  assert(total_loop.total_real_cyc > total_loop.total_event_values[1]);

  assert(single_cycle.total_real_cyc > 0);
  assert(single_cycle.total_real_usec > 0);
  assert(single_cycle.total_event_values[0] > 0);
  assert(single_cycle.total_event_values[1] > 0);
  assert(single_cycle.total_times_called == itercount);
  assert(single_cycle.total_real_cyc > single_cycle.total_event_values[1]);

  //Also the outer loop values should be approximately the same as the accumulated inner loop values.
  // At the moment a 5% error will be considered acceptable
  assert(relative_error(total_loop.total_event_values[1], single_cycle.total_event_values[1]) < 0.01);
  assert(relative_error(total_loop.total_event_values[0], single_cycle.total_event_values[0]) < 0.01);
  assert(relative_error(total_loop.total_real_cyc, single_cycle.total_real_cyc) < 0.01);
  assert(relative_error(total_loop.total_real_usec, single_cycle.total_real_usec) < 0.01);

  stopwatch_destroy();
}

int main() {
  test_stopwatch_perf_mat_mul();
  test_stopwatch_perf_mat_mul_loop();
}

