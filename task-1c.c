#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int sum_ints(int* nums, int size) {
    int acc = 0;
    for (int i = 0; i < size; ++i) {
        acc += nums[i];
    }
    return acc;
}

int main(int argc, char** argv) {
    int n;
    if (argc > 1) {
        n = atoi(argv[1]);
    } else {
        printf("n is needed\n");
        return 0;
    }
    
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
        
        MPI_Request req;
        MPI_Isend(nums, n, MPI_INT, 0, 0, MPI_COMM_WORLD, &req);
        MPI_Status stat;
        MPI_Wait(&req, &stat);
        printf("proc %d sent nums with sum=%d to %d\n", rank, sum_ints(nums, n), 0);
        free(nums);

    } else {
        int maxsum = -1;
        int maxsum_rank = -1;

        MPI_Request* reqs = calloc(size - 1, sizeof(MPI_Request));
        int* nums = calloc(size - 1, n * sizeof(int));
        for (int i = 1; i < size; ++i) {
            MPI_Irecv(nums + n * (i - 1), n, MPI_INT, i, MPI_ANY_TAG, MPI_COMM_WORLD, reqs + i - 1);
        }
        MPI_Status* stats = calloc(size - 1, sizeof(MPI_Status));
        MPI_Waitall(size - 1, reqs, stats);
        free(reqs);
        free(stats);

        for (int i = 1; i < size; ++i) {
            int sum = sum_ints(nums + n * (i - 1), n);
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
