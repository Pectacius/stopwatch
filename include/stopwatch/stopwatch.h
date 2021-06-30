#ifndef STOPWATCH_STOPWATCH_H
#define STOPWATCH_STOPWATCH_H

// structure to hold the measurements
struct Measurements {
  long long real_cyc_elapsed_since_start;
  long long real_usec_elapsed_since_start;
  long long l1_misses_since_start;
  long long cyc_wait_resource_since_start;
};

// Initializes the stopwatch structure. Currently the events that are measured are hard coded.
int init_stopwatch();

// Starts the timer. Meant to be a monotonic clock
int start_stopwatch();

// Stops the monotonic clock
int stop_stopwatch();

// Cleans up resources used by the timer. Interestingly valgrind still reports a memory leak with the PAPI resources
int destroy_stopwatch();

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
int read_stopwatch(struct Measurements *reading);

// Performs a delta between each value and prints out the results
void print_results(struct Measurements *start, struct Measurements *end);

#endif //STOPWATCH_STOPWATCH_H
