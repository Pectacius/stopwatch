#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "stopwatch/stopwatch.h"

// Row-major multiplication (traditional C ordering)
void row_major(int N, float A[N][N], float B[N][N], float C[N][N]);

// Column-major multiplication (Fortran ordering, but backwards for C)
void column_major(int N, float A[N][N], float B[N][N], float C[N][N]);

int main() {
  int N = 1000;
  int itercount = 10;

  struct Measurements *row_iter_start = malloc(sizeof(struct Measurements) * itercount);
  struct Measurements *row_iter_end = malloc(sizeof(struct Measurements) * itercount);
  struct Measurements *col_iter_start = malloc(sizeof(struct Measurements) * itercount);
  struct Measurements *col_iter_end = malloc(sizeof(struct Measurements) * itercount);

  int ret_val = init_stopwatch();
  if (ret_val != 0) {
    printf("Error initializing stopwatch\n");
    return -1;
  }

  ret_val = start_stopwatch();
  if (ret_val != 0) {
    printf("Error starting stopwatch\n");
    return -1;
  }

  // row-major
  {
    float (*A)[N], (*B)[N], (*C)[N];

    A = calloc(sizeof(float), N * N);
    B = calloc(sizeof(float), N * N);
    C = calloc(sizeof(float), N * N);

    for (int ii = 0; ii < N; ii++) {
      for (int jj = 0; jj < N; jj++) {
        A[ii][jj] = ii + jj;
        B[ii][jj] = ii * jj;
      }
    }

    for (int iter = 0; iter < itercount; iter++) {
      // clear C array
      memset(C, 0, sizeof(float) * N * N);
      ret_val = read_stopwatch(&row_iter_start[iter]);
      if (ret_val != 0) {
        printf("Error reading measurement\n");
        return -1;
      }
      // Perform the multiplication
      row_major(N, A, B, C);
      ret_val = read_stopwatch(&row_iter_end[iter]);
      if (ret_val != 0) {
        printf("Error reading measurement\n");
        return -1;
      }
    }

    free(A);
    free(B);
    free(C);
  }
  printf("Completed row major iterations\n");
  // column-major
  {
    float (*A)[N], (*B)[N], (*C)[N];

    A = calloc(sizeof(float), N * N);
    B = calloc(sizeof(float), N * N);
    C = calloc(sizeof(float), N * N);

    for (int jj = 0; jj < N; jj++) {
      for (int ii = 0; ii < N; ii++) {
        A[jj][ii] = ii + jj;
        B[jj][ii] = ii * jj;
      }
    }

    for (int iter = 0; iter < itercount; iter++) {
      // clear C array
      memset(C, 0, sizeof(float) * N * N);

      ret_val = read_stopwatch(&col_iter_start[iter]);
      if (ret_val != 0) {
        printf("Error reading measurement\n");
        return -1;
      }
      // Perform the multiplication
      column_major(N, A, B, C);
      ret_val = read_stopwatch(&col_iter_end[iter]);
      if (ret_val != 0) {
        printf("Error reading measurement\n");
        return -1;
      }
    }
  }
  printf("Completed column major iterations\n");
  // write results to file
  FILE *fpt = fopen("row_output.csv", "w");

  fprintf(fpt, "row_real_cyc,row_real_usec,row_l1_miss,row_cyc_wait_resource\n");
  for (int i = 0; i < itercount; i++) {
    fprintf(fpt, "%lld,%lld,%lld,%lld\n",
            row_iter_end[i].real_cyc_elapsed_since_start - row_iter_start[i].real_cyc_elapsed_since_start,
            row_iter_end[i].real_usec_elapsed_since_start - row_iter_start[i].real_usec_elapsed_since_start,
            row_iter_end[i].l1_misses_since_start - row_iter_start[i].l1_misses_since_start,
            row_iter_end[i].cyc_wait_resource_since_start - row_iter_start[i].cyc_wait_resource_since_start);
  }
  fclose(fpt);

  fpt = fopen("col_output.csv", "w");

  fprintf(fpt, "col_real_cyc,col_real_usec,col_l1_miss,col_cyc_wait_resource\n");

  for (int i = 0; i < itercount; i++) {
    fprintf(fpt, "%lld,%lld,%lld,%lld\n",
            col_iter_end[i].real_cyc_elapsed_since_start - col_iter_start[i].real_cyc_elapsed_since_start,
            col_iter_end[i].real_usec_elapsed_since_start - col_iter_start[i].real_usec_elapsed_since_start,
            col_iter_end[i].l1_misses_since_start - col_iter_start[i].l1_misses_since_start,
            col_iter_end[i].cyc_wait_resource_since_start - col_iter_start[i].cyc_wait_resource_since_start);
  }
}

// Matrix multiplication with row-major arrays
// see https://en.wikipedia.org/wiki/Matrix_multiplication_algorithm#Iterative_algorithm
// for the algorithm itself
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

// Matrix multiplication with column-major arrays (reversed indices)
void column_major(int N, float A[N][N], float B[N][N], float C[N][N]) {
  for (int row = 0; row < N; row++) {
    for (int col = 0; col < N; col++) {
      // Take the product of A[row,*] and B[*,col] to set C[row,col]
      for (int ii = 0; ii < N; ii++) {
        C[col][row] += A[ii][row] * B[col][ii];
      }
    }
  }
}

