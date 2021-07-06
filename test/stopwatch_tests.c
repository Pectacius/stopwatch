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

// Tests the constructor for a StopwatchReadings. ALl fields should be initially set to zero.
void test_stopwatch_create_stopwatch_readings() {
  struct StopwatchReadings test_readings = create_stopwatch_readings();

  assert(test_readings.total_real_cyc == 0);
  assert(test_readings.total_real_usec == 0);
  assert(test_readings.total_l1_misses == 0);
  assert(test_readings.total_cyc_wait_resource == 0);

  assert(test_readings.total_times_called == 0);

  assert(test_readings.start_real_cyc == 0);
  assert(test_readings.start_real_usec == 0);
  assert(test_readings.start_l1_misses == 0);
  assert(test_readings.start_cyc_wait_resource == 0);
}

// Does a simple performance measurement on a matrix multiplication
void test_stopwatch_perf_mat_mul() {
  int N = 1000; // Square matrix size
  struct StopwatchReadings test_readings = create_stopwatch_readings();

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
  assert(record_start_measurements(&test_readings) == STOPWATCH_OK);
  row_major(N, A, B, C);
  assert(record_end_measurements(&test_readings) == STOPWATCH_OK);

  // check results. At the moment there is not exactly a precise way to assert that the produced value is accurate due
  // to various factors such as hardware specs and randomness from the OS. Hence these assertions at the moment will
  // only check that the values are not absurd i.e the total time is not 0 etc, until a method is devised that can
  // accurately check these values while also being hardware independent.
  assert(test_readings.total_real_cyc > 0);
  assert(test_readings.total_real_usec > 0);
  assert(test_readings.total_l1_misses > 0);
  assert(test_readings.total_cyc_wait_resource > 0);

  assert(test_readings.total_times_called == 1);

  // the total amount of cpu cycles should always be greater than the cycles waiting for resources, otherwise the cpu
  // does no work which cannot be as floating point operations are being performed.
  assert(test_readings.total_real_cyc > test_readings.total_cyc_wait_resource);

  assert(destroy_event_timers() == STOPWATCH_OK);
}

// Repeatedly does the same matrix multiplication to test the accumulation feature
void test_stopwatch_perf_mat_mul_loop() {
  int N = 1000; // Square matrix size
  int iter_count = 10; // Number of times to multiply

  struct StopwatchReadings test_readings_outer = create_stopwatch_readings(); // measurement for outside of the loop
  struct StopwatchReadings
      test_readings_inner = create_stopwatch_readings(); // accumulated measurements for inside of loop

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

  assert(record_start_measurements(&test_readings_outer) == STOPWATCH_OK);
  for (int i = 0; i < iter_count; i++) {
    memset(C, 0, sizeof(float) * N * N);

    assert(record_start_measurements(&test_readings_inner) == STOPWATCH_OK);
    row_major(N, A, B, C);
    assert(record_end_measurements(&test_readings_inner) == STOPWATCH_OK);
  }
  assert(record_end_measurements(&test_readings_outer) == STOPWATCH_OK);

  assert(destroy_event_timers() == STOPWATCH_OK);

  // Check results. See previous comment on how assertions are made.
  assert(test_readings_outer.total_real_cyc > 0);
  assert(test_readings_outer.total_real_usec > 0);
  assert(test_readings_outer.total_l1_misses > 0);
  assert(test_readings_outer.total_cyc_wait_resource > 0);
  assert(test_readings_outer.total_times_called == 1);
  assert(test_readings_outer.total_real_cyc > test_readings_outer.total_cyc_wait_resource);

  assert(test_readings_inner.total_real_cyc > 0);
  assert(test_readings_inner.total_real_usec > 0);
  assert(test_readings_inner.total_l1_misses > 0);
  assert(test_readings_inner.total_cyc_wait_resource > 0);
  assert(test_readings_inner.total_times_called == iter_count);
  assert(test_readings_inner.total_real_cyc > test_readings_inner.total_cyc_wait_resource);



  //Also the outer loop values should be approximately the same as the accumulated inner loop values.
  // At the moment a 5% error will be considered acceptable
  assert(
      relative_error(test_readings_outer.total_cyc_wait_resource, test_readings_inner.total_cyc_wait_resource) < 0.05);
  assert(
      relative_error(test_readings_outer.total_l1_misses, test_readings_inner.total_l1_misses) < 0.05);
  assert(
      relative_error(test_readings_outer.total_real_cyc, test_readings_inner.total_real_cyc) < 0.05);
  assert(
      relative_error(test_readings_outer.total_real_usec, test_readings_inner.total_real_usec) < 0.05);

}

int main() {
  test_stopwatch_setup_teardown();
  test_stopwatch_setup_teardown_multiple_times();
  test_stopwatch_setup_twice();
  test_stopwatch_teardown_twice();
  test_stopwatch_create_stopwatch_readings();
  test_stopwatch_perf_mat_mul();
  test_stopwatch_perf_mat_mul_loop();
}

