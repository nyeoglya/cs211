/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    if (M == 32 && N == 32) {
        unsigned i, j, ib, jb; // base variable
        unsigned tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7; // temp variable
        for (ib=0; ib<N; ib+=8) {
            for (jb=0; jb<M; jb+=8) {
                if (ib == jb) { // diagonal
                    for (i=ib; i<ib+8; i++) {
                        // optimization using the temp vars.
                        tmp0 = A[i][jb+0];
                        tmp1 = A[i][jb+1];
                        tmp2 = A[i][jb+2];
                        tmp3 = A[i][jb+3];
                        tmp4 = A[i][jb+4];
                        tmp5 = A[i][jb+5];
                        tmp6 = A[i][jb+6];
                        tmp7 = A[i][jb+7];
                        B[jb+0][i] = tmp0;
                        B[jb+1][i] = tmp1;
                        B[jb+2][i] = tmp2;
                        B[jb+3][i] = tmp3;
                        B[jb+4][i] = tmp4;
                        B[jb+5][i] = tmp5;
                        B[jb+6][i] = tmp6;
                        B[jb+7][i] = tmp7;
                    }
                } else { // non-diagonal
                    for (i=ib; i<ib+8; i++) {
                        for (j=jb; j<jb+8; j++) {
                            B[j][i] = A[i][j]; // manual block matrix transpose
                        }
                    }
                }
            }
        }
    } else if (M == 64 && N == 64) {
        unsigned i, k, ib, jb; // base variable
        unsigned tmp0, tmp1, tmp2, tmp3; // temp variable

        for (ib = 0; ib < N; ib += 8) {
            for (jb = 0; jb < M; jb += 8) {
                // first 4 rows
                for (i = ib; i < ib + 4; i++) {
                    // upper-left matrix transpose
                    tmp0 = A[i][jb+0];
                    tmp1 = A[i][jb+1];
                    tmp2 = A[i][jb+2];
                    tmp3 = A[i][jb+3];
                    B[jb+0][i] = tmp0;
                    B[jb+1][i] = tmp1;
                    B[jb+2][i] = tmp2;
                    B[jb+3][i] = tmp3;

                    // upper-right matrix transpose
                    tmp0 = A[i][jb+4];
                    tmp1 = A[i][jb+5];
                    tmp2 = A[i][jb+6];
                    tmp3 = A[i][jb+7];
                    B[jb+0][i+4] = tmp0;
                    B[jb+1][i+4] = tmp1;
                    B[jb+2][i+4] = tmp2;
                    B[jb+3][i+4] = tmp3;
                }

                // upperright & lowerleft
                for (k = 0; k < 4; k++) {
                    // retrieve upperright value of B
                    tmp0 = B[jb+k][ib+4];
                    tmp1 = B[jb+k][ib+5];
                    tmp2 = B[jb+k][ib+6];
                    tmp3 = B[jb+k][ib+7];

                    // transpose lowerleft A to upperright B
                    B[jb+k][ib+4] = A[ib+4][jb+k];
                    B[jb+k][ib+5] = A[ib+5][jb+k];
                    B[jb+k][ib+6] = A[ib+6][jb+k];
                    B[jb+k][ib+7] = A[ib+7][jb+k];

                    // save temp values to the lowerleft position
                    B[jb+k+4][ib+0] = tmp0;
                    B[jb+k+4][ib+1] = tmp1;
                    B[jb+k+4][ib+2] = tmp2;
                    B[jb+k+4][ib+3] = tmp3;
                }

                // last 4 rows
                for (i = ib + 4; i < ib + 8; i++) {
                    // lower-right matrix transpose
                    tmp0 = A[i][jb+4];
                    tmp1 = A[i][jb+5];
                    tmp2 = A[i][jb+6];
                    tmp3 = A[i][jb+7];
                    B[jb+4][i] = tmp0;
                    B[jb+5][i] = tmp1;
                    B[jb+6][i] = tmp2;
                    B[jb+7][i] = tmp3;
                }
            }
        }
    } else if (M == 61 && N == 67) {
        unsigned i, j, ib, jb, bs=14; // base variable
        for (ib=0; ib<N; ib+=bs) { // manual block matrix transpose
            for (jb=0; jb<M; jb+=bs) {
                for (i=ib; i<ib+bs && i<N; i++) { // resolve edge cases (M, N is not a 16*x form)
                    for (j=jb; j<jb+bs && j<M; j++) {
                        B[j][i] = A[i][j];
                    }
                }
            }
        }
    }
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

