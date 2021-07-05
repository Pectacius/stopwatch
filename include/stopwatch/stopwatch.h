#ifndef STOPWATCH_STOPWATCH_H
#define STOPWATCH_STOPWATCH_H

#define STOPWATCH_OK 0
#define STOPWATCH_ERR -1

// Structure used to hold readings from the measurement clock.
struct StopwatchReadings {
  long long total_real_cyc;
  long long total_real_usec;
  long long total_l1_misses;
  long long total_cyc_wait_resource;

  long long total_times_called;

  long long start_real_cyc;
  long long start_real_usec;
  long long start_l1_misses;
  long long start_cyc_wait_resource;
};

//======================================================================================================================
// Monotonic clock initialization and destruction
//======================================================================================================================

// Initializes the event timers. Currently the events that are measured are hard coded. This will also start the
// monotonic measurement clock as currently it is assumed that consumers would immediately start the clock after
// initializing the stopwatch structure.
int init_event_timers();

// Stops the monotonic event timers and cleans up resources used by the timer. Interestingly valgrind still reports a
// memory leak with the PAPI specific resources
int destroy_event_timers();

// Creates a stopwatch reading with default values
struct StopwatchReadings create_stopwatch_readings();

//======================================================================================================================
// Operations on StopwatchReadings
//======================================================================================================================

// Records the current values on the monotonic event timers
int record_start_measurements(struct StopwatchReadings* readings);

// Records the current values on the monotonic event timers. Will also perform a delta between the values recorded from
// `record_start_measurements` and the current values to give a measurement on the profile on the procedure executing
// between the calls of `record_start_measurements` and `record_end_measurements`
int record_end_measurements(struct StopwatchReadings* readings);

// Prints out the results
void print_results(struct StopwatchReadings *readings);

#endif //STOPWATCH_STOPWATCH_H
