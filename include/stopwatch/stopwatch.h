#ifndef STOPWATCH_STOPWATCH_H
#define STOPWATCH_STOPWATCH_H

#include <stddef.h>
// Function return statuses
enum StopwatchStatus {
  STOPWATCH_OK,
  STOPWATCH_TOO_MANY_EVENTS,
  STOPWATCH_INVALID_EVENT,
  STOPWATCH_INVALID_EVENT_COMB,
  STOPWATCH_INVALID_FILE,
  STOPWATCH_ERR,
};

#define STOPWATCH_MAX_EVENTS 10
#define NULL_TERM_MAX_ROUTINE_NAME_LEN 16

// =====================================================================================================================
// Structure holding results for a specific entry
// =====================================================================================================================
struct StopwatchMeasurementResult {
  long long total_real_usec;
  long long total_event_values[STOPWATCH_MAX_EVENTS];
  long long total_times_called;
  char routine_name[NULL_TERM_MAX_ROUTINE_NAME_LEN];
  size_t caller_routine_id;
  size_t num_of_events;
  int event_names[STOPWATCH_MAX_EVENTS];
};

// =====================================================================================================================
// Monotonic clock initialization and destruction
// =====================================================================================================================
// Initializes the event timers. Currently the events that are measured are hard coded. This will also start the
// monotonic measurement clock as currently it is assumed that consumers would immediately start the clock after
// initializing the stopwatch structure.
enum StopwatchStatus stopwatch_init();

// Stops the monotonic event timers and cleans up resources used by the timer. Interestingly valgrind still reports a
// memory leak with the PAPI specific resources
void stopwatch_destroy();

// =====================================================================================================================
// Operations
// =====================================================================================================================

// Records the current values on the monotonic event timers
enum StopwatchStatus stopwatch_record_start_measurements(size_t routine_id, const char *function_name, size_t caller_routine_id);

// Records the current values on the monotonic event timers. Will also perform a delta between the values recorded from
// `stopwatch_record_start_measurements` and the current values to give a measurement on the profile on the procedure executing
// between the calls of `stopwatch_record_start_measurements` and `stopwatch_record_end_measurements`
enum StopwatchStatus stopwatch_record_end_measurements(size_t routine_id);

enum StopwatchStatus stopwatch_get_measurement_results(size_t routine_id, struct StopwatchMeasurementResult *result);

// Prints out the results
void stopwatch_print_measurement_results(struct StopwatchMeasurementResult *result);

// Generates a table as a pretty printed string
void stopwatch_print_result_table();

// Saves results to specified file
enum StopwatchStatus stopwatch_result_to_csv(const char* file_name);

#endif //STOPWATCH_STOPWATCH_H
