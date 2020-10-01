#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int min_int(int left, int right) {
    if (left < right) {
        return left;
    } else {
        return right;
    }
}

int main(int argc, char** argv) {
    int n = 7;
    int comm_rank, comm_size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &comm_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
    comm_size = min_int(comm_size, n);
    if (comm_rank >= comm_size) {
        MPI_Finalize();
        return 0;
    }
    srand(comm_rank + 1);

    int prev_comm_rank;
    if (comm_rank == 0) {
        prev_comm_rank = comm_size - 1;
    } else {
        prev_comm_rank = comm_rank - 1;
    }
    int next_comm_rank = (comm_rank + 1) % comm_size;
    int acc = 0;
    for (int i = 0;; ++i) {
        int count = i * comm_size + comm_rank;
        if (count >= n) {
            break;
        }

        if (count > 0) {
            MPI_Status status;
            MPI_Recv(&acc, 1, MPI_INT, prev_comm_rank, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            printf("proc %d received acc=%d from %d\n", comm_rank, acc, prev_comm_rank);
        }

        int rnd = rand() % 101;
        acc += rnd;
        printf("proc %d added %d to acc=%d\n", comm_rank, rnd, acc - rnd);

        if (count == n - 1) {
            printf("sum=%d\n", acc);
            break;
        }

        MPI_Send(&acc, 1, MPI_INT, next_comm_rank, 0, MPI_COMM_WORLD);
        printf("proc %d sent acc=%d to %d\n", comm_rank, acc, next_comm_rank);
    }

    MPI_Finalize();
    return 0;
}
