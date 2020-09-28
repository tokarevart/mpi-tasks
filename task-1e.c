#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <mpi.h>

int min(int left, int right) {
    if (left < right) {
        return left;
    } else {
        return right;
    }
}

double random() {
    static double inv_half_rand_max = 2.0 / RAND_MAX;
    return inv_half_rand_max * rand() - 1.0;
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

double std_dev(double* nums, double mean, int size) {
    double acc = 0.0;
    for (int i = 0; i < size; ++i) {
        double dev = nums[i] - mean;
        acc += dev * dev;
    }
    return sqrt(acc / (size - 1));
}

struct IntBlock{
    int beg;
    int end;
    int size;
};

struct IntBlock partition(int size, int num_blocks, int block_idx) {
    num_blocks = min(num_blocks, size);
    int block_maxsize = (size - 1) / num_blocks + 1;
    int block_beg = block_idx * block_maxsize;
    int block_end = min(block_beg + block_maxsize, size);
    struct IntBlock res = { block_beg, block_end, block_end - block_beg };
    return res;
}

double run_task(int comm_rank, int comm_size, int n, int q) {
    struct IntBlock block = partition(q, comm_size, comm_rank);

    double* means = calloc(block.size, sizeof(double));
    double* cur_nums = calloc(n, sizeof(double));
    for (int i = 0; i < block.size; ++i) {
        fill_with_randoms(cur_nums, n);
        means[i] = sum_dbls(cur_nums, n) / n;
    }
    free(cur_nums);

    if (comm_rank > 0) {
        MPI_Send(means, block.size, MPI_INT, 0, 0, MPI_COMM_WORLD);
        free(means);
        return 0.0;

    } else {
        double start = MPI_Wtime();

        double* loc_means = means;
        double* means = calloc(q, sizeof(double));
        memcpy(means, loc_means, block.size * sizeof(double));
        free(loc_means);

        double means_mean = 0.0;
        double* recv_dest = means + block.size;
        for (int i = 1; i < comm_size; ++i) {
            struct IntBlock block = partition(q, comm_size, i);
            MPI_Status status;
            MPI_Recv(recv_dest, block.size, MPI_INT, i, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            means_mean += sum_dbls(recv_dest, block.size);
            recv_dest += block.size;
        }
        means_mean /= q;

        double stddev = std_dev(means, means_mean, q);

        double elapsed = MPI_Wtime() - start;

        printf("%f", stddev);
        return elapsed;
    }
}

int main(int argc, char** argv) {
    if (argc < 3) {
        printf("n and q is needed\n");
        return 0;
    }
    int n = atoi(argv[1]);
    int q = atoi(argv[2]);
    
    int comm_rank, comm_size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &comm_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
    if (comm_rank >= q) {
        MPI_Finalize();
        return 0;
    }
    comm_size = min(comm_size, q);
    srand(comm_rank);
    
    double min_runtime = __DBL_MAX__;
    for (int i = 0; i < 1; ++i) {
        MPI_Barrier(MPI_COMM_WORLD);
        double elapsed = run_task(comm_rank, comm_size, n, q);
        if (elapsed < min_runtime) {
            min_runtime = elapsed;
        }
    }

    MPI_Finalize();
    return 0;
}
