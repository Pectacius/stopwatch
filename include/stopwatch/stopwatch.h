#ifndef STOPWATCH_STOPWATCH_H
#define STOPWATCH_STOPWATCH_H

// structure to hold the measurements
struct Measurements {
  long long real_cyc_elapsed_since_start;
  long long real_usec_elapsed_since_start;
  long long l1_misses_since_start;
  long long cyc_wait_resource_since_start;
};

int init_stopwatch();

int start_stopwatch();

int stop_stopwatch();

int destroy_stopwatch();

int read_stopwatch(struct Measurements *reading);

void print_results(struct Measurements *start, struct Measurements *end);

#endif //STOPWATCH_STOPWATCH_H
