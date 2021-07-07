// Multiplies matrices row wise multiple times and measures statistics for each iteration
#include "stopwatch/stopwatch.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void row_major(int N, float A[N][N], float B[N][N], float C[N][N]);

int main() {
  if (init_event_timers() != STOPWATCH_OK) {
    printf("Error initializing stopwatch\n");
    exit(-1);
  }

  {
    int N = 500;
    int itercount = 10;

    // Structure for results
    struct MeasurementResult result;

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

    if (record_start_measurements(1, "total-loop", 0) != STOPWATCH_OK) {
      printf("Error reading measurements\n");
      exit(-1);
    }

    for (int iter = 0; iter < itercount; iter++) {
      // clear C array
      memset(C, 0, sizeof(float) * N * N);

      // read start time
      if (record_start_measurements(2, "single-cycle", 1) != STOPWATCH_OK) {
        printf("Error reading measurements\n");
        exit(-1);
      }
      // Perform the multiplication
      row_major(N, A, B, C);

      // read end time
      if (record_end_measurements(2) != STOPWATCH_OK) {
        printf("Error reading measurements\n");
        exit(-1);
      }
    }

    if (record_end_measurements(1) != 0) {
      printf("Error reading measurements\n");
      exit(-1);
    }

    free(A);
    free(B);
    free(C);

    get_measurement_results(1, &result);
    print_measurement_results(&result);
    printf("\n");

    get_measurement_results(2, &result);
    print_measurement_results(&result);
  }

  if (destroy_event_timers() != STOPWATCH_OK) {
    printf("Error cleaning up stopwatch\n");
    exit(-1);
  }
}

void row_major(int N, float A[N][N], float B[N][N], float C[N][N]) {
  for (int row = 0; row < N; row++) {
    for (int col = 0; col < N; col++) {
      // Take the product of A[row,*] and B[*,col] to set C[row,col]
      for (int ii = 0; ii < N; ii++) {
        C[row][col] += A[row][ii] * B[ii][col];
      }
    }
  }
}

