#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <memory.h>
#include <mpi.h>

double random() {
    static double inv_rand_max = 1.0 / RAND_MAX;
    return inv_rand_max * rand();
}

void fill_with_randoms(double* nums, int size) {
    for (int i = 0; i < size; ++i) {
        nums[i] = random();
    }
}

double sum_dbls(double* nums, int size) {
    double acc = 0.0;
    for (int i = 0; i < size; ++i) {
        acc += nums[i];
    }
    return acc;
}

void run_scatter(int comm_rank, int comm_size, int n) {

}

void run_gather(int comm_rank, int comm_size, int n) {

}

void run_allgather(int comm_rank, int comm_size, int n) {

}

void run_alltoall(int comm_rank, int comm_size, int n) {

}

int main(int argc, char** argv) {
    int n = 10;

    int comm_rank, comm_size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &comm_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);

    srand(1);
    run_bcast(comm_rank, comm_size, n);

    srand(1);
    run_scatter(comm_rank, comm_size, n);

    srand(1);
    run_gather(comm_rank, comm_size, n);

    srand(1);
    run_allgather(comm_rank, comm_size, n);

    srand(1);
    run_alltoall(comm_rank, comm_size, n);

    MPI_Finalize();
    return 0;
}
