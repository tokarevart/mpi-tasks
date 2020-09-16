#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(int argc, char** argv) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank > 0) {
        srand(rank);
        int rnd = rand() % 101;
        MPI_Send(&rnd, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        printf("proc %d sent %d to %d\n", rank, rnd, 0);
    } else {
        for (int i = 1; i < size; ++i) {
            int rnd;
            MPI_Status status;
            MPI_Recv(&rnd, 1, MPI_INT, i, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            printf("proc %d received %d from %d\n", rank, rnd, i);
        }
    }

    MPI_Finalize();
    return 0;
}
