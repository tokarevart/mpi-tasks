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
    double* nums_cs_times = calloc(n * comm_size, sizeof(double));
    fill_with_randoms(nums_cs_times, n);
    for (int i = 1; i < comm_size; ++i) {
        memcpy(nums_cs_times + i * n, nums_cs_times, n * sizeof(double));
    }
    printf("[scatter]   proc %d sent   sum=%f\n", comm_rank, sum_dbls(nums_cs_times, n));

    double* buf = calloc(n, sizeof(double));
    for (int i = 0; i < comm_size; ++i) {
        MPI_Scatter(nums_cs_times, n, MPI_DOUBLE, buf, n, MPI_DOUBLE, i, MPI_COMM_WORLD);
        if (comm_rank != i) {
            printf("[scatter]   proc %d recved sum=%f from %d\n", comm_rank, sum_dbls(buf, n), i);
        }
    }
    
    free(nums_cs_times);
    free(buf);
}

void run_gather(int comm_rank, int comm_size, int n) {
    double* nums = calloc(n, sizeof(double));
    fill_with_randoms(nums, n);
    printf("[gather]    proc %d sent   sum=%f\n", comm_rank, sum_dbls(nums, n));

    double* buf = calloc(n * comm_size, sizeof(double));
    // the abbreviations tx and rx are used for transmitter and receiver respectively (tradition in many fields)
    for (int rx = 0; rx < comm_size; ++rx) {
        MPI_Gather(nums, n, MPI_DOUBLE, buf, n, MPI_DOUBLE, rx, MPI_COMM_WORLD);
        if (comm_rank != rx) continue;
        for (int tx = 0; tx < comm_size; ++tx) {
            if (tx == rx) continue;
            printf("[gather]    proc %d recved sum=%f from %d\n", rx, sum_dbls(buf + tx * n, n), tx);            
        }
    }

    free(nums);
    free(buf);
}

void run_allgather(int comm_rank, int comm_size, int n) {
    double* nums = calloc(n, sizeof(double));
    fill_with_randoms(nums, n);
    printf("[allgather] proc %d sent   sum=%f\n", comm_rank, sum_dbls(nums, n));

    double* buf = calloc(n * comm_size, sizeof(double));
    MPI_Allgather(nums, n, MPI_DOUBLE, buf, n, MPI_DOUBLE, MPI_COMM_WORLD);
    
    for (int tx = 0; tx < comm_size; ++tx) {
        if (comm_rank == tx) continue;
        printf("[allgather] proc %d recved sum=%f from %d\n", comm_rank, sum_dbls(buf + tx * n, n), tx);
    }
    
    free(nums);
    free(buf);
}

void run_alltoall(int comm_rank, int comm_size, int n) {
    double* nums_cs_times = calloc(n * comm_size, sizeof(double));
    fill_with_randoms(nums_cs_times, n);
    for (int i = 1; i < comm_size; ++i) {
        memcpy(nums_cs_times + i * n, nums_cs_times, n * sizeof(double));
    }
    printf("[alltoall]  proc %d sent   sum=%f\n", comm_rank, sum_dbls(nums_cs_times, n));

    double* buf = calloc(n * comm_size, sizeof(double));
    MPI_Alltoall(nums_cs_times, n, MPI_DOUBLE, buf, n, MPI_DOUBLE, MPI_COMM_WORLD);
    
    for (int tx = 0; tx < comm_size; ++tx) {
        if (comm_rank == tx) continue;
        printf("[alltoall]  proc %d recved sum=%f from %d\n", comm_rank, sum_dbls(buf + tx * n, n), tx);
    }
    
    free(nums_cs_times);
    free(buf);
}

int main(int argc, char** argv) {
    int n = 10;

    int comm_rank, comm_size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &comm_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);

    srand(comm_rank + 1);
    run_scatter(comm_rank, comm_size, n);

    srand(comm_rank + 1);
    run_gather(comm_rank, comm_size, n);

    srand(comm_rank + 1);
    run_allgather(comm_rank, comm_size, n);

    srand(comm_rank + 1);
    run_alltoall(comm_rank, comm_size, n);

    MPI_Finalize();
    return 0;
}
