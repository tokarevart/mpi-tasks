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

void run_allreduce(int comm_rank, int comm_size, int n) {
    int block_size = (n - 1) / comm_size + 1;
    double* block = calloc(block_size, sizeof(double));
    double* allnums;
    if (comm_rank == 0) {
        allnums = calloc(block_size * comm_size, sizeof(double));
        fill_with_randoms(allnums, n);
        for (int i = n; i < block_size * comm_size; ++i) {
            allnums[i] = 0.0;
        }
        printf("[allreduce]        seq sum=%f\n", sum_dbls(allnums, n));
    }
    MPI_Scatter(allnums, block_size, MPI_DOUBLE, block, block_size, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    double* buf = calloc(block_size, sizeof(double));
    MPI_Allreduce(block, buf, block_size, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
    printf("[allreduce] proc %d par sum=%f\n", comm_rank, sum_dbls(buf, block_size));

    if (comm_rank == 0) {
        free(allnums);
    }
    free(buf);
}

void run_reduce(int comm_rank, int comm_size, int n) {
    int block_size = (n - 1) / comm_size + 1;
    double* block = calloc(block_size, sizeof(double));
    double* allnums;
    if (comm_rank == 0) {
        allnums = calloc(block_size * comm_size, sizeof(double));
        fill_with_randoms(allnums, n);
        for (int i = n; i < block_size * comm_size; ++i) {
            allnums[i] = 0.0;
        }
        printf("[reduce]           seq sum=%f\n", sum_dbls(allnums, n));
    }
    MPI_Scatter(allnums, block_size, MPI_DOUBLE, block, block_size, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    double* buf = calloc(block_size, sizeof(double));
    MPI_Reduce(block, buf, block_size, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Bcast(buf, block_size, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    printf("[reduce]    proc %d par sum=%f\n", comm_rank, sum_dbls(buf, block_size));

    if (comm_rank == 0) {
        free(allnums);
    }
    free(buf);
}

int main(int argc, char** argv) {
    int n = 100;

    int comm_rank, comm_size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &comm_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);

    srand(1);
    run_allreduce(comm_rank, comm_size, n);

    srand(1);
    run_reduce(comm_rank, comm_size, n);

    MPI_Finalize();
    return 0;
}
