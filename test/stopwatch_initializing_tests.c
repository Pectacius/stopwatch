// Basic testing for the stopwatch library
// Simple way to unittest with only using asserts
// Currently there is a subtle issue where each test is not isolated from each other.
#include "stopwatch/stopwatch.h"
#include <assert.h>


// Initialize and destroy the event timers once
void test_stopwatch_setup_teardown() {
  const enum StopwatchEvents events[] = {TOTAL_CYCLES, L1_CACHE_MISS};
  const unsigned int num_events = 2;
  assert(stopwatch_init(events, num_events) == STOPWATCH_OK);
  stopwatch_destroy();
}

// Initialize and destroy the event timers multiple times. This will assure that paired calls to `init_event_timers` and
// `destroy_event_timers` should always succeed.
void test_stopwatch_setup_teardown_multiple_times() {
  const enum StopwatchEvents events[] = {TOTAL_CYCLES, L1_CACHE_MISS};
  const unsigned int num_events = 2;
// Call `init_event_timer` and `destroy_event_timer`
  const int max_iterations = 10;
  for (int i = 0; i < max_iterations; i++) {
    assert(stopwatch_init(events, num_events) == STOPWATCH_OK);
    stopwatch_destroy();
  }
}

// This test case aims to mimic the situation where the event timers have already be initialized and there is another
// call to initialize the event timers.
void test_stopwatch_setup_twice() {
  const enum StopwatchEvents events[] = {TOTAL_CYCLES, L1_CACHE_MISS};
  const unsigned int num_events = 2;
  assert(stopwatch_init(events, num_events) == STOPWATCH_OK);
  assert(stopwatch_init(events, num_events) == STOPWATCH_ERR);
  stopwatch_destroy();
}

// Tests that the initial total times called is set to zero in the `MeasurementReadings` struct. All the other fields
// will be set to their default values but that should not matter as they will be overridden anyways.
void test_stopwatch_times_called_initial_value() {
  struct StopwatchMeasurementResult result;
  for (unsigned int idx = 0; idx < 500; idx++) {
    assert(stopwatch_get_measurement_results(idx, &result) == STOPWATCH_OK);
    assert(result.total_times_called == 0);
  }
}

int main() {
  test_stopwatch_setup_teardown();
  test_stopwatch_setup_teardown_multiple_times();
  test_stopwatch_setup_twice();
  //test_stopwatch_teardown_twice();
  test_stopwatch_times_called_initial_value();
}

