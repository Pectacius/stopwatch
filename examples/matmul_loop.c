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

    // Structures for holding measurements
    struct StopwatchReadings row_major_total = create_stopwatch_readings();
    struct StopwatchReadings row_major_single = create_stopwatch_readings();

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

    if (record_start_measurements(&row_major_total) != STOPWATCH_OK) {
      printf("Error reading measurements\n");
      exit(-1);
    }

    for (int iter = 0; iter < itercount; iter++) {
      // clear C array
      memset(C, 0, sizeof(float) * N * N);

      // read start time
      if (record_start_measurements(&row_major_single) != STOPWATCH_OK) {
        printf("Error reading measurements\n");
        exit(-1);
      }
      // Perform the multiplication
      row_major(N, A, B, C);

      // read end time
      if (record_end_measurements(&row_major_single) != STOPWATCH_OK) {
        printf("Error reading measurements\n");
        exit(-1);
      }
    }

    if (record_end_measurements(&row_major_total) != 0) {
      printf("Error reading measurements\n");
      exit(-1);
    }

    free(A);
    free(B);
    free(C);

    printf("%d iterations of %dx%d row major matrix multiplication\n", itercount, N, N);
    printf("Total loop measurements\n");
    print_results(&row_major_total);
    printf("Accumulated cycles measurements\n");
    print_results(&row_major_single);
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

