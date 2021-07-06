// Utility matrix multiplication functions to be used in stopwatch testing

#ifndef LIBSTOPWATCH_TEST_MAT_MUL_H_
#define LIBSTOPWATCH_TEST_MAT_MUL_H_

// Row major matrix multiplication
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

#endif //LIBSTOPWATCH_TEST_MAT_MUL_H_
