#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stopwatch/stopwatch.h"

// Prototypes of matrix multiplication functions

// Row-major multiplication (traditional C ordering)
void row_major(int N, float A[N][N], float B[N][N], float C[N][N]);

// Column-major multiplication (Fortran ordering, but backwards for C)
void column_major(int N, float A[N][N], float B[N][N], float C[N][N]);

// Array-of-pointers
void pointer_major(int N, float* A[N], float* B[N], float* C[N]);

// Recursive, cache-oblivious algorithm
void cache_oblivious(int N, float A[N][N], float B[N][N], float C[N][N]);

// Breakout function for recursion
void _cache_oblivious(int N, float A[N][N], float B[N][N], float C[N][N],
                      int clb_i, int cub_i, int clb_j, int cub_j,
                      int alb_i, int aub_i, int alb_j, int aub_j,
                      int blb_i, int bub_i, int blb_j, int bub_j);


int main()
{
    int N = 1000; // Number of rows/columns in matrix to multiply
    int itercount = 1; // Number of iterations for timing

    int ret_val = init_stopwatch();
    if (ret_val != 0) {
        printf("Error initializing stopwatch\n");
        return -1;
    }
    struct Measurements* result = malloc(sizeof(struct Measurements));

    // Conventional, row-major multiplication, A*B = C
    {
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

        ret_val = start_measurement();
        if (ret_val != 0) {
            printf("Error starting measurements\n");
            return -1;
        }

        for (int iter = 0; iter < itercount; iter++) {
            // clear C array
            memset(C, 0, sizeof(float) * N * N);

            // Perform the multiplication
            row_major(N, A, B, C);
        }
        ret_val = stop_measurement(result);
        if (ret_val != 0) {
            printf("Error stopping measurements\n");
            return -1;
        }

        free(A);
        free(B);
        free(C);

        printf("%d iterations of %dx%d row major matrix multiplication\n", itercount, N, N);
        print_results(result);
    }

    // Conventional, column-major multiplication, A*B = C
    {
        // Allocate the A, B, and C arrays on the heap.
        // Note that the meaning of each index is reversed from the row-major version above, but this
        // does not change the memory allocations since the matrices are square.
        float (*A)[N], (*B)[N], (*C)[N];

        // Use calloc for the allocation to initialize the memory to 0
        A = calloc(sizeof(float), N*N);
        B = calloc(sizeof(float), N*N);
        C = calloc(sizeof(float), N*N);

        // Initialize A and B to token values: i+j and i*j.  Memory accesses are base-zero.
        // These matrices are now accessed in the "wrong" order.
        for (int jj = 0; jj < N; jj++) {
            for (int ii = 0; ii < N; ii++) {
                A[jj][ii] = ii+jj;
                B[jj][ii] = ii*jj;
            }
        }

        start_measurement();

        for (int iter = 0; iter < itercount; iter++) {
            // clear C array
            memset(C,0,sizeof(float)*N*N);

            // Perform the multiplication
            column_major(N,A,B,C);
        }

        stop_measurement(result);

        // Clean up memory
        free(A);
        free(B);
        free(C);

        printf("%d iterations of %dx%d column major matrix multiplication\n", itercount, N, N);
        print_results(result);
    }

    // "Pointer-major" mutliplication
    // Some C tutorials recommend allocating 2D arrays as a 1D array of 1D arrays (an array of pointers).
    // This example demonstrates the performance impact
    {
        // Allocate the A, B, and C arrays as an array of pointers.  Note that this differs from the previous
        // exmaple only via punctuation, which is a syntactic wart of C
        float * A[N], * B[N], * C[N];

        // Use calloc for the allocation to initialize the memory to 0.  This time, we must loop over each
        // subarray since they are allocated independently
        for (int i = 0; i < N; i++) {
            A[i] = calloc(sizeof(float), N);
            B[i] = calloc(sizeof(float), N);
            C[i] = calloc(sizeof(float), N);
        }

        // Initialize A and B to token values: i+j and i*j.
        for (int jj = 0; jj < N; jj++) {
            for (int ii = 0; ii < N; ii++) {
                A[ii][jj] = ii+jj;
                B[ii][jj] = ii*jj;
            }
        }

        start_measurement();

        for (int iter = 0; iter < itercount; iter++) {
            // clear C array
            for (int i = 0; i < N; i++) {
                memset(C[i],0,sizeof(float)*N);
            }

            // Perform the multiplication
            pointer_major(N,A,B,C);
        }

        stop_measurement(result);

        // Clean up memory
        for (int i = 0; i < N; i++) {
            free(A[i]);
            free(B[i]);
            free(C[i]);
        }

        printf("%d iterations of %dx%d pointer major matrix multiplication\n", itercount, N, N);
        print_results(result);
    }

    // Cache-oblivious matrix multiplication; as row-major, but using the cache-oblivious multiplier
    {
        // Allocate the A, B, and C arrays on the heap.
        float (*A)[N], (*B)[N], (*C)[N];

        // Use calloc for the allocation to initialize the memory to 0
        A = calloc(sizeof(float), N*N);
        B = calloc(sizeof(float), N*N);
        C = calloc(sizeof(float), N*N);

        // Initialize A and B to token values: i+j and i*j.  Memory accesses are base-zero.
        for (int jj = 0; jj < N; jj++) {
            for (int ii = 0; ii < N; ii++) {
                A[ii][jj] = ii+jj;
                B[ii][jj] = ii*jj;
            }
        }

        start_measurement();

        for (int iter = 0; iter < itercount; iter++) {
            // clear C array
            memset(C,0,sizeof(float)*N*N);

            // Perform the multiplication
            cache_oblivious(N,A,B,C);
        }

        stop_measurement(result);

        // Clean up memory
        free(A);
        free(B);
        free(C);

        printf("%d iterations of %dx%d cache oblivious matrix multiplication\n", itercount, N, N);
        print_results(result);
    }

    return 0;
}

// Matrix multiplication with row-major arrays
// see https://en.wikipedia.org/wiki/Matrix_multiplication_algorithm#Iterative_algorithm
// for the algorithm itself
void row_major(int N, float A[N][N], float B[N][N], float C[N][N]) {
    for (int row = 0; row < N; row++) {
        for (int col = 0; col < N; col++) {
            // Take the product of A[row,*] and B[*,col] to set C[row,col]
            for (int ii = 0; ii < N; ii++) {
                C[row][col] += A[row][ii]*B[ii][col];
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
                C[col][row] += A[ii][row]*B[col][ii];
            }
        }
    }
}

// "Pointer" major, using an array-of-pointers formulation
void pointer_major(int N, float* A[N], float* B[N], float* C[N]) {
    for (int row = 0; row < N; row++) {
        for (int col = 0; col < N; col++) {
            // Take the product of A[row,*] and B[*,col] to set C[row,col]
            for (int ii = 0; ii < N; ii++) {
                C[row][col] += A[row][ii]*B[ii][col];
            }
        }
    }
}

// "Cache-oblivious" matrix multiplication
// This algorithm is fundamentally recursive, and it takes advantage of the simplification
// [A B] * [E F] = [AE + BG, AF + BH]
// [C D]   [G H]   [CE + DG, CF + DH]
// which holds true when A, B, etc are both individual numbers and sub-matrices.  By recursively
// breaking the full martix down into smaller matrix multiplications, the smaller multiplications
// are more cache-efficient.  This algorithm is "cache-oblivious" because it retains its efficiency
// no matter the size of the CPU cache hierarchy, whereas algorithms that break the matrix into
// discrete blocks must have a block size specified a priori.

// Interface function
void cache_oblivious(int N, float A[N][N], float B[N][N], float C[N][N]) {
    // Call the function that does the real work
    _cache_oblivious(N, A, B, C, 0, N-1, 0, N-1,
                     0, N-1, 0, N-1,
                     0, N-1, 0, N-1);
}

#define CACHE_BLOCK 8
void _cache_oblivious(int N, float A[N][N], float B[N][N], float C[N][N],
                      int clb_i, int cub_i, int clb_j, int cub_j,
                      int alb_i, int aub_i, int alb_j, int aub_j,
                      int blb_i, int bub_i, int blb_j, int bub_j) {
    /* The additional function parameters here select a subset of A/B/C for multiplication.
       The submatrix C[clb_i:cub_i, clb_j:cub_j] will be updated with the product
       A[alb_i:aub_i,alb_j:aub_j]*B[blb_i:bub_i,blb_j:bub_j]

       As a naming convention, 'lb' is lower bound and 'ub' is upper bound, _i and _j
       are the first and second indices, and these bounds are inclusive. */

    // First, ensure that the dimensions given are compatible.  For non power of 2 sizes,
    // the recursive step will generate rectangular matrix multiplications
    //printf("A: %d-%d x %d-%d\n",alb_i,aub_i,alb_j,aub_j);
    //printf("B: %d-%d x %d-%d\n",blb_i,bub_i,blb_j,bub_j);
    //printf("C: %d-%d x %d-%d\n",clb_i,cub_i,clb_j,cub_j);
    assert(aub_j-alb_j == bub_i - blb_i); // cols(A) == rows(B)
    assert(aub_i-alb_i == cub_i - clb_i); // rows(A) == rows(C)
    assert(bub_j-blb_j == cub_j - clb_j); // cols(B) == cols(C)

    /* Base case: perform the matrix multiplication directly if the region is small enough.
       Define "small enough" as "16 or fewer rows/columns of output" */
    if (cub_i - clb_i < CACHE_BLOCK || cub_j - clb_j < CACHE_BLOCK) {
        for (int row = 0; row <= cub_i - clb_i; row++) {
            for (int col = 0; col <= cub_j - clb_j ; col++) {
                for (int ii = 0; ii <= aub_j - alb_j; ii++) {
                    C[row+clb_i][col+clb_j] += A[row+alb_i][ii+alb_j]*B[ii+blb_i][col+blb_j];
                }
            }
        }
    } else {
        /* Otherwise, break this multiplication up into 8 sub-multiplications */
        int cmid_i = (clb_i + cub_i)/2,
                cmid_j = (clb_j + cub_j)/2,
                amid_i = (alb_i + aub_i)/2,
                amid_j = (alb_j + aub_j)/2,
                bmid_i = (blb_i + bub_i)/2,
                bmid_j = (blb_j + bub_j)/2;

        // Reminder:
        // [A B] * [E F] = [AE + BG, AF + BH]
        // [C D]   [G H]   [CE + DG, CF + DH]

        // Top-left, AE
        //printf("AE\n");
        _cache_oblivious(N,A,B,C,clb_i,cmid_i,clb_j,cmid_j, // top-left
                         alb_i,amid_i,alb_j,amid_j, // A
                         blb_i,bmid_i,blb_j,bmid_j); // E
        // Top-left, BG
        //printf("BG\n");
        _cache_oblivious(N,A,B,C,clb_i,cmid_i,clb_j,cmid_j, // top-left
                         alb_i,amid_i,amid_j+1,aub_j, // B
                         bmid_i+1,bub_i,blb_j,bmid_j); // G
        // Top-right, AF
        //printf("AF\n");
        _cache_oblivious(N,A,B,C,clb_i,cmid_i,cmid_j+1,cub_j, // top-right
                         alb_i,amid_i,alb_j,amid_j, // A
                         blb_i,bmid_i,bmid_j+1,bub_j); // F
        // Top-right, BH
        //printf("BH\n");
        _cache_oblivious(N,A,B,C,clb_i,cmid_i,cmid_j+1,cub_j, // top-right
                         alb_i,amid_i,amid_j+1,aub_j, // B
                         bmid_i+1,bub_i,bmid_j+1,bub_j); // H
        // Bottom-left, CE
        //printf("CE\n");
        _cache_oblivious(N,A,B,C,cmid_i+1,cub_i,clb_j,cmid_j, // bottom-left
                         amid_i+1,aub_i,alb_j,amid_j, // C
                         blb_i,bmid_i,blb_j,bmid_j); // E
        // Bottom-left, DG
        //printf("DG\n");
        _cache_oblivious(N,A,B,C,cmid_i+1,cub_i,clb_j,cmid_j, // bottom-left
                         amid_i+1,aub_i,amid_j+1,aub_j, // D
                         bmid_i+1,bub_i,blb_j,bmid_j); // G
        // Bottom-right, CF
        //printf("CF\n");
        _cache_oblivious(N,A,B,C,cmid_i+1,cub_i,cmid_j+1,cub_j, // bottom-right
                         amid_i+1,aub_i,alb_j,amid_j, // C
                         blb_i,bmid_i,bmid_j+1,bub_j); // F
        // Bottom-right, DH
        //printf("DH\n");
        _cache_oblivious(N,A,B,C,cmid_i+1,cub_i,cmid_j+1,cub_j, // bottom-right
                         amid_i+1,aub_i,amid_j+1,aub_j, // D
                         bmid_i+1,bub_i,bmid_j+1,bub_j); // H
    }
}

