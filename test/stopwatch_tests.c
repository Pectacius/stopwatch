// Basic testing for the stopwatch library
// Simple way to unittest with only using asserts
// Currently there is a subtle issue where each test is not isolated from each other.
#include "stopwatch/stopwatch.h"
#include "mat_mul.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

double relative_error(long long val1, long long val2) {
  return fabs((double) (val1 - val2) / (double) val1);
}

// Initialize and destroy the event timers once
void test_stopwatch_setup_teardown() {
  assert(init_event_timers() == STOPWATCH_OK);
  assert(destroy_event_timers() == STOPWATCH_OK);
}

// Initialize and destroy the event timers multiple times. This will assure that paired calls to `init_event_timers` and
// `destroy_event_timers` should always succeed.
void test_stopwatch_setup_teardown_multiple_times() {
// Call `init_event_timer` and `destroy_event_timer`
  const int max_iterations = 10;
  for (int i = 0; i < max_iterations; i++) {
    assert(init_event_timers() == STOPWATCH_OK);
    assert(destroy_event_timers() == STOPWATCH_OK);
  }
}

// This test case aims to mimic the situation where the event timers have already be initialized and there is another
// call to initialize the event timers.
void test_stopwatch_setup_twice() {
  assert(init_event_timers() == STOPWATCH_OK); // The first call should execute as normal
  assert(init_event_timers() == STOPWATCH_ERR); // The second call should execute with an error
  assert(destroy_event_timers() == STOPWATCH_OK); // Teardown should execute as normal
}

// This test case aims to mimic the situation where the event timers are no longer initialized
void test_stopwatch_teardown_twice() {
  assert(init_event_timers() == STOPWATCH_OK); // The first call should execute as normal
  assert(destroy_event_timers() == STOPWATCH_OK); // Teardown should execute as normal
  assert(destroy_event_timers() == STOPWATCH_ERR); // The second call to teardown should execute with an error
}

// Tests that the initial total times called is set to zero in the `MeasurementReadings` struct. All the other fields
// will be set to their default values but that should not matter as they will be overridden anyways.
void test_stopwatch_times_called_initial_value() {
  struct MeasurementResult result;
  for (unsigned int idx = 0; idx < 500; idx++) {
    assert(get_measurement_results(idx, &result) == STOPWATCH_OK);
    assert(result.total_times_called == 0);
  }
}

// Does a simple performance measurement on a matrix multiplication
void test_stopwatch_perf_mat_mul() {
  int N = 1000; // Square matrix size
  struct MeasurementResult result;

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

  assert(init_event_timers() == STOPWATCH_OK);
  assert(record_start_measurements(1, "mat-mul", 0) == STOPWATCH_OK);
  row_major(N, A, B, C);
  assert(record_end_measurements(1) == STOPWATCH_OK);

  // Check the function name and stack depth are correct
  assert(get_measurement_results(1, &result) == STOPWATCH_OK);
  assert(result.stack_depth == 0);
  assert(strcmp(result.routine_name, "mat-mul") == 0);

  // check results. At the moment there is not exactly a precise way to assert that the produced value is accurate due
  // to various factors such as hardware specs and randomness from the OS. Hence these assertions at the moment will
  // only check that the values are not absurd i.e the total time is not 0 etc, until a method is devised that can
  // accurately check these values while also being hardware independent.
  assert(result.total_real_cyc > 0);
  assert(result.total_real_usec > 0);
  assert(result.total_l1_misses > 0);
  assert(result.total_cyc_wait_resource > 0);

  assert(result.total_times_called == 1);

  // the total amount of cpu cycles should always be greater than the cycles waiting for resources, otherwise the cpu
  // does no work which cannot be as floating point operations are being performed.
  assert(result.total_real_cyc > result.total_cyc_wait_resource);

  assert(destroy_event_timers() == STOPWATCH_OK);
}

// Repeatedly does the same matrix multiplication to test the accumulation feature
void test_stopwatch_perf_mat_mul_loop() {
  int N = 1000; // Square matrix size
  int iter_count = 10; // Number of times to multiply

  float (*A)[N] = calloc(sizeof(float), N * N);
  float (*B)[N] = calloc(sizeof(float), N * N);
  float (*C)[N] = calloc(sizeof(float), N * N);

  for (int jj = 0; jj < N; jj++) {
    for (int ii = 0; ii < N; ii++) {
      A[ii][jj] = ii + jj;
      B[ii][jj] = ii * jj;
    }
  }
  assert(init_event_timers() == STOPWATCH_OK);

  assert(record_start_measurements(1, "total-loop", 0) == STOPWATCH_OK);
  for (int i = 0; i < iter_count; i++) {
    memset(C, 0, sizeof(float) * N * N);

    assert(record_start_measurements(2, "single-cycle", 1) == STOPWATCH_OK);
    row_major(N, A, B, C);
    assert(record_end_measurements(2) == STOPWATCH_OK);
  }
  assert(record_end_measurements(1) == STOPWATCH_OK);

  assert(destroy_event_timers() == STOPWATCH_OK);

  struct MeasurementResult total_loop;
  struct MeasurementResult single_cycle;

  assert(get_measurement_results(1, &total_loop) == STOPWATCH_OK);
  assert(total_loop.stack_depth == 0);
  assert(strcmp(total_loop.routine_name, "total-loop") == 0);

  assert(get_measurement_results(2, &single_cycle) == STOPWATCH_OK);
  assert(single_cycle.stack_depth == 1);
  assert(strcmp(single_cycle.routine_name, "single-cycle") == 0);

  // Check results. See previous comment on how assertions are made.
  assert(total_loop.total_real_cyc > 0);
  assert(total_loop.total_real_usec > 0);
  assert(total_loop.total_l1_misses > 0);
  assert(total_loop.total_cyc_wait_resource > 0);
  assert(total_loop.total_times_called == 1);
  assert(total_loop.total_real_cyc > total_loop.total_cyc_wait_resource);

  assert(single_cycle.total_real_cyc > 0);
  assert(single_cycle.total_real_usec > 0);
  assert(single_cycle.total_l1_misses > 0);
  assert(single_cycle.total_cyc_wait_resource > 0);
  assert(single_cycle.total_times_called == iter_count);
  assert(single_cycle.total_real_cyc > single_cycle.total_cyc_wait_resource);

  //Also the outer loop values should be approximately the same as the accumulated inner loop values.
  // At the moment a 5% error will be considered acceptable
  assert(relative_error(total_loop.total_cyc_wait_resource, single_cycle.total_cyc_wait_resource) < 0.001);
  assert(relative_error(total_loop.total_l1_misses, single_cycle.total_l1_misses) < 0.001);
  assert(relative_error(total_loop.total_real_cyc, single_cycle.total_real_cyc) < 0.001);
  assert(relative_error(total_loop.total_real_usec, single_cycle.total_real_usec) < 0.001);
}

int main() {
  test_stopwatch_setup_teardown();
  test_stopwatch_setup_teardown_multiple_times();
  test_stopwatch_setup_twice();
  test_stopwatch_teardown_twice();
  test_stopwatch_times_called_initial_value();
  test_stopwatch_perf_mat_mul();
  test_stopwatch_perf_mat_mul_loop();
}

