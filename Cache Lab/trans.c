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
    // values that should be stored in the registers
    int r0, r1, r2, r3;
    int r4, r5, r6, r7;
    // values serve as indices and sometimes serve as temp variables
    int i, j, k, l;
    // case 1
    // The matrix splits into 8 x 8 blocks
    if ((M == 32) && (N == 32))
    {
        // for off-diagonal blocks, take transpositions directly
        for (i = 0; i < N; i += 8)
            for (j = 0; j < M; j += 8)
                if (i != j)
                    for (k = 0; k < 8; k++)
                        for (l = 0; l < 8; l++)
                           B[i + k][j + l] = A[j + l][i + k];
        // for diagonal blocks, firstly copy blocks to B
        for (i = 0; i < N; i += 8)
            for (k = 0; k < 8; k++)
            {
                r0 = A[i + k][i + 0];
                r1 = A[i + k][i + 1];
                r2 = A[i + k][i + 2];
                r3 = A[i + k][i + 3];
                r4 = A[i + k][i + 4];
                r5 = A[i + k][i + 5];
                r6 = A[i + k][i + 6];
                r7 = A[i + k][i + 7];
                B[i + k][i + 0] = r0;
                B[i + k][i + 1] = r1;
                B[i + k][i + 2] = r2;
                B[i + k][i + 3] = r3;
                B[i + k][i + 4] = r4;
                B[i + k][i + 5] = r5;
                B[i + k][i + 6] = r6;
                B[i + k][i + 7] = r7;
            }
        // then take transpositions in B directly
        for (i = 0; i < N; i += 8)
            for (k = 0; k < 8; k++)
                for (l = k + 1; l < 8; l++)
                {
                    j = B[i + k][i + l]; //now j serves as a temp local variable
                    B[i + k][i + l] = B[i + l][i + k];
                    B[i + l][i + k] = j;
                }
    }
    // case 2
    // The matrix splits into 8 x 8 blocks again, but at this time every four rows shares the same cache address
    if ((M == 64) && (N == 64))
    {
        // for a fixed block, given the first four rows, we split them into two 4 x 4 blocks,
        // and then we take transpositions inside these two blocks independently
        // Note that after this, the left-top block is in the correct position but the right-top is not
        for (i = 0; i < N; i += 8)
            for (j = 0; j < M; j += 8)
            {
                for (k = 0; k < 4; k++)
                {
                    r0 = A[i + k][j + 0];
                    r1 = A[i + k][j + 1];
                    r2 = A[i + k][j + 2];
                    r3 = A[i + k][j + 3];
                    r4 = A[i + k][j + 4];
                    r5 = A[i + k][j + 5];
                    r6 = A[i + k][j + 6];
                    r7 = A[i + k][j + 7];
                    B[j + 0][i + k] = r0;
                    B[j + 1][i + k] = r1;
                    B[j + 2][i + k] = r2;
                    B[j + 3][i + k] = r3;
                    B[j + 0][i + k + 4] = r4;
                    B[j + 1][i + k + 4] = r5;
                    B[j + 2][i + k + 4] = r6;
                    B[j + 3][i + k + 4] = r7;
                }
                // for the remaining four rows, we split them into two 4 x 4 blocks again,
                // and then we take the first columns in these two blocks and move them to the correct position in B
                // in the meantime we can move the first row in the right-top block to the correct position,
                // and then we deal with the second columns and so on

                for (l = 0; l < 4; l++)
                {
                    r0 = A[i + 4][j + l];
                    r1 = A[i + 5][j + l];
                    r2 = A[i + 6][j + l];
                    r3 = A[i + 7][j + l];
                    r4 = A[i + 4][j + l + 4];
                    r5 = A[i + 5][j + l + 4];
                    r6 = A[i + 6][j + l + 4];
                    r7 = A[i + 7][j + l + 4];
                    //now k serves as a temp local local variable
                    k = B[j + l][i + 4];
                    B[j + l][i + 4] = r0;
                    r0 = k;

                    k = B[j + l][i + 5];
                    B[j + l][i + 5] = r1;
                    r1 = k;

                    k = B[j + l][i + 6];
                    B[j + l][i + 6] = r2;
                    r2 = k;

                    k = B[j + l][i + 7];
                    B[j + l][i + 7] = r3;
                    r3 = k;

                    B[j + l + 4][i + 0] = r0;
                    B[j + l + 4][i + 1] = r1;
                    B[j + l + 4][i + 2] = r2;
                    B[j + l + 4][i + 3] = r3;
                    B[j + l + 4][i + 4] = r4;
                    B[j + l + 4][i + 5] = r5;
                    B[j + l + 4][i + 6] = r6;
                    B[j + l + 4][i + 7] = r7;
                }
            }
    }
    // case 3
    // we split the 64 x 56 submatrix into 8 x 8 blocks and apply the method in case 2 to this submatrix
    if ((M == 61) && (N == 67))
    {
        for (i = 0; i < 64; i += 8)
            for (j = 0; j < 56; j += 8)
            {
                for (k = 0; k < 4; k++)
                {
                    r0 = A[i + k][j + 0];
                    r1 = A[i + k][j + 1];
                    r2 = A[i + k][j + 2];
                    r3 = A[i + k][j + 3];
                    r4 = A[i + k][j + 4];
                    r5 = A[i + k][j + 5];
                    r6 = A[i + k][j + 6];
                    r7 = A[i + k][j + 7];
                    B[j + 0][i + k] = r0;
                    B[j + 1][i + k] = r1;
                    B[j + 2][i + k] = r2;
                    B[j + 3][i + k] = r3;
                    B[j + 0][i + k + 4] = r4;
                    B[j + 1][i + k + 4] = r5;
                    B[j + 2][i + k + 4] = r6;
                    B[j + 3][i + k + 4] = r7;
                }
                for (l = 0; l < 4; l++)
                {
                    r0 = A[i + 4][j + l];
                    r1 = A[i + 5][j + l];
                    r2 = A[i + 6][j + l];
                    r3 = A[i + 7][j + l];
                    r4 = A[i + 4][j + l + 4];
                    r5 = A[i + 5][j + l + 4];
                    r6 = A[i + 6][j + l + 4];
                    r7 = A[i + 7][j + l + 4];
                    //now k serves as a temp local local variable
                    k = B[j + l][i + 4];
                    B[j + l][i + 4] = r0;
                    r0 = k;

                    k = B[j + l][i + 5];
                    B[j + l][i + 5] = r1;
                    r1 = k;

                    k = B[j + l][i + 6];
                    B[j + l][i + 6] = r2;
                    r2 = k;

                    k = B[j + l][i + 7];
                    B[j + l][i + 7] = r3;
                    r3 = k;

                    B[j + l + 4][i + 0] = r0;
                    B[j + l + 4][i + 1] = r1;
                    B[j + l + 4][i + 2] = r2;
                    B[j + l + 4][i + 3] = r3;
                    B[j + l + 4][i + 4] = r4;
                    B[j + l + 4][i + 5] = r5;
                    B[j + l + 4][i + 6] = r6;
                    B[j + l + 4][i + 7] = r7;
                }
            }
        // for remaining entires, we store as many as possible values in the registers and move them
        // to the correct positions
        for (i = 0; i < N; i++)
        {
            r0 = A[i][56];
            r1 = A[i][57];
            r2 = A[i][58];
            r3 = A[i][59];
            r4 = A[i][60];
            B[56][i] = r0;
            B[57][i] = r1;
            B[58][i] = r2;
            B[59][i] = r3;
            B[60][i] = r4;
        }
        for (j = 0; j < 56; j++)
        {
            r5 = A[64][j];
            r6 = A[65][j];
            r7 = A[66][j];
            B[j][64] = r5;
            B[j][65] = r6;
            B[j][66] = r7;
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

