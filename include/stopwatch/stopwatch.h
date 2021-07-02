#ifndef STOPWATCH_STOPWATCH_H
#define STOPWATCH_STOPWATCH_H

// Structure used to hold readings from the measurement clock.
struct StopwatchReadings {
  long long real_cyc;
  long long real_usec;
  long long l1_misses;
  long long cyc_wait_resource;
};

//======================================================================================================================
// Monotonic clock initialization and destruction
//======================================================================================================================

// Initializes the stopwatch structure. Currently the events that are measured are hard coded. This will also start the
// monotonic measurement clock as currently it is assumed that consumers would immediately start the clock after
// initializing the stopwatch structure.
int init_stopwatch();

// Stops the monotonic clock and cleans up resources used by the timer. Interestingly valgrind still reports a memory
// leak with the PAPI specific resources
int destroy_stopwatch();

//======================================================================================================================
// Operations on StopwatchReadings
//======================================================================================================================

// Gets the current time from the monotonic clock. Ideally, the usage would be reading the value before timing and
// reading it again after timing and using the delta as a total elapsed measurement.
// Measuring a function `foo` would proceed like so.
//      struct Measurements* start_structure = malloc(sizeof(struct Measurements));
//      struct Measurements* end_structure = malloc(sizeof(struct Measurements));
//      read_stopwatch(start_structure);
//      foo();
//      read_stopwatch(end_structure);
//
// Performing a difference between the values in `start_structure` and `end_structure` will produce the actual
// measurement for the execution of `foo`.
int read_stopwatch(struct StopwatchReadings *reading);

// Performs a delta between each value and prints out the results
void print_results(struct StopwatchReadings *start, struct StopwatchReadings *end);

#endif //STOPWATCH_STOPWATCH_H
