#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <mpi.h>

int sum_ints(int* nums, int size) {
    int acc = 0;
    for (int i = 0; i < size; ++i) {
        acc += nums[i];
    }
    return acc;
}

int main(int argc, char** argv) {
    int n = 50;
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank > 0) {
        srand(rank);
        int* nums = calloc(n, sizeof(int));
        for (int i = 0; i < n; ++i) {
            nums[i] = rand() % 100;
        }
        
        MPI_Send(nums, n, MPI_INT, 0, 0, MPI_COMM_WORLD);
        free(nums);
        printf("proc %d sent nums with sum=%d to %d\n", rank, sum_ints(nums, n), 0);

    } else {
        int maxsum = -1;
        int maxsum_rank = -1;

        int* nums = calloc(n, sizeof(int));
        for (int i = 1; i < size; ++i) {
            MPI_Status status;
            MPI_Recv(nums, n, MPI_INT, i, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

            int sum = sum_ints(nums, n);
            if (sum > maxsum) {
                maxsum = sum;
                maxsum_rank = i;
            }

            printf("proc %d received nums with sum=%d from %d\n", rank, sum, i);
        }
        free(nums);

        printf("nums with max sum=%d received from %d\n", maxsum, maxsum_rank);
    }

    MPI_Finalize();
    return 0;
}
