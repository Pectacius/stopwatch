#ifndef STOPWATCH_STOPWATCH_H
#define STOPWATCH_STOPWATCH_H

// structure to hold the measurements
struct Measurements{
    long long real_cyc_elapsed;
    long long real_usec_elapsed;
    long long virt_cyc_elapsed;
    long long virt_usec_elapsed;
    long long l1_misses;
    long long cyc_wait_resource;
};

int init_stopwatch();

int start_measurement();

int stop_measurement(struct Measurements* result);

void print_results(struct Measurements* result);

#endif //STOPWATCH_STOPWATCH_H
