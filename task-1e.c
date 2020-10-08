#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <mpi.h>

int min_int(int left, int right) {
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

double std_dev(double* nums, int size, double mean) {
    double acc = 0.0;
    for (int i = 0; i < size; ++i) {
        double dev = nums[i] - mean;
        acc += dev * dev;
    }
    return sqrt(acc / (size - 1));
}

typedef struct {
    int beg;
    int end;
    int size;
} IntBlock;

// constraint: block_idx < min(total, num_blocks) 
IntBlock partition(int total, int num_blocks, int block_idx) {
    num_blocks = min_int(num_blocks, total);
    int block_maxsize = (total - 1) / num_blocks + 1;
    int block_beg = block_idx * block_maxsize;
    int block_end = min_int(block_beg + block_maxsize, total);
    IntBlock res = { block_beg, block_end, block_end - block_beg };
    return res;
}

typedef struct {
    double stddev;
    double runtime;
} TaskRes;

TaskRes run_task(int comm_rank, int comm_size, int n, int q) {
    double start = MPI_Wtime();

    IntBlock block = partition(q, comm_size, comm_rank);

    double* means = calloc(block.size, sizeof(double));
    double* cur_nums = calloc(n, sizeof(double));
    for (int i = 0; i < block.size; ++i) {
        fill_with_randoms(cur_nums, n);
        means[i] = sum_dbls(cur_nums, n) / n;
    }
    free(cur_nums);

    if (comm_rank > 0) {
        MPI_Send(means, block.size, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
        free(means);

        TaskRes res;
        return res;

    } else {
        double* loc_means = means;
        double* means = calloc(q, sizeof(double));
        memcpy(means, loc_means, block.size * sizeof(double));
        free(loc_means);

        double means_mean = 0.0;
        double* recv_dest = means + block.size;
        for (int i = 1; i < comm_size; ++i) {
            IntBlock block = partition(q, comm_size, i);
            MPI_Status status;
            MPI_Recv(recv_dest, block.size, MPI_DOUBLE, i, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            means_mean += sum_dbls(recv_dest, block.size);
            recv_dest += block.size;
        }
        means_mean /= q;

        double stddev = std_dev(means, q, means_mean);
        free(means);
        double elapsed = MPI_Wtime() - start;
        TaskRes res = { stddev, elapsed };
        return res;
    }
}

int main(int argc, char** argv) {
    if (argc < 4) {
        printf("outopt, n and q is needed\n");
        return 0;
    }
    char* outopt = argv[1];
    int n = atoi(argv[2]);
    int q = atoi(argv[3]);
    if (strcmp(outopt, "runtime") != 0 &&
        strcmp(outopt, "stddev") != 0) {
        printf("outopt must be 'runtime' or 'stddev'");
        return 0;
    }
    
    int comm_rank, comm_size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &comm_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
    if (comm_rank >= q) {
        MPI_Finalize();
        return 0;
    }
    comm_size = min_int(comm_size, q);
    srand(comm_rank);
    
    TaskRes res;
    if (strcmp(outopt, "runtime") == 0) {
        res.runtime = __DBL_MAX__;
        for (int i = 0; i < 20; ++i) {
            MPI_Barrier(MPI_COMM_WORLD);
            TaskRes cur_res = run_task(comm_rank, comm_size, n, q);
            if (cur_res.runtime < res.runtime) {
                res = cur_res;
            }
        }
    } else {
        res = run_task(comm_rank, comm_size, n, q);
    }
    
    if (comm_rank == 0) {
        if (strcmp(outopt, "runtime") == 0) {
            printf("%d\t%f\n", comm_size, res.runtime);
        } else {
            printf("%d\t%f\n", q, res.stddev);
        }
    }

    MPI_Finalize();
    return 0;
}
