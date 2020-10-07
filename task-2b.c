#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <mpi.h>

int min_int(int left, int right) {
    if (left < right) {
        return left;
    } else {
        return right;
    }
}

bool is_odd(int idx) {
    return idx % 2 == 1;
}

bool is_even(int idx) {
    return idx % 2 == 0;
}

typedef enum {
    FORWARD,
    REVERSE
} Direction;

int circular_inc(int idx, int size) {
    return (idx + 1) % size;
}

int circular_dec(int idx, int size) {
    if (idx == 0) {
        return size - 1;
    } else {
        return idx - 1;
    }
}

typedef bool (*Filter)(int);

int circular_inc_with_filter(int idx, int size, Filter filter) {
    for (int i = 0; i < size; ++i) {
        idx = circular_inc(idx, size);
        if (filter(idx)) {
            return idx;
        }
    }
    return -1;
}

int circular_dec_with_filter(int idx, int size, Filter filter) {
    for (int i = 0; i < size; ++i) {
        idx = circular_dec(idx, size);
        if (filter(idx)) {
            return idx;
        }
    }
    return -1;
}

int pos_in_filtered_seq(int idx, int size, Filter filter) {
    int pos = 0;
    for (int i = 0; i < size; ++i) {
        if (!filter(i)) {
            continue;
        }
        if (i == idx) {
            return pos;
        }
        ++pos;
    }
    return -1;
}

int filtered_seq_size(int size, Filter filter) {
    int first = -1;
    for (int i = 0; i < size; ++i) {
        if (filter(i)) {
            first = i;
            break;
        }
    }
    if (first == -1) {
        return 0;
    }

    int cur = first;
    for (int i = 0; i < size; ++i) {
        cur = circular_inc(cur, size);
        if (filter(cur) && cur == first) {
            return i + 1;
        }
    }
    return -1;
}

void run_task_with_rank_filter(Filter filter, Direction dir, int comm_rank, int comm_size, int n) {
    if (!filter(comm_rank)) {
        printf("run_task_with_rank_filter fn got wrong comm_rank\n");
        return;
    }

    char dirmark;
    if (dir == FORWARD) {
        dirmark = 'F';
    } else {
        dirmark = 'R';
    }

    int prev_comm_rank;
    int next_comm_rank;
    if (dir == FORWARD) {
        prev_comm_rank = circular_dec_with_filter(comm_rank, comm_size, filter);
        next_comm_rank = circular_inc_with_filter(comm_rank, comm_size, filter);
    } else {
        prev_comm_rank = circular_inc_with_filter(comm_rank, comm_size, filter);
        next_comm_rank = circular_dec_with_filter(comm_rank, comm_size, filter);
    }

    int seq_size = filtered_seq_size(comm_size, filter);
    int acc = 0;
    for (int i = 0;; ++i) {
        int count = i * seq_size + pos_in_filtered_seq(comm_rank, comm_size, filter);
        if (count >= n) {
            break;
        }

        MPI_Request req;
        MPI_Status status;
        if (count > 0) {
            MPI_Irecv(&acc, 1, MPI_INT, prev_comm_rank, MPI_ANY_TAG, MPI_COMM_WORLD, &req);
            MPI_Wait(&req, &status);
            printf("[%c] proc %d received acc=%d from %d\n", dirmark, comm_rank, acc, prev_comm_rank);
        }

        int rnd = rand() % 101;
        acc += rnd;
        printf("[%c] proc %d added %d to acc=%d\n", dirmark, comm_rank, rnd, acc - rnd);

        if (count == n - 1) {
            printf("[%c] sum=%d\n", dirmark, acc);
            break;
        }

        MPI_Isend(&acc, 1, MPI_INT, next_comm_rank, 0, MPI_COMM_WORLD, &req);
        MPI_Wait(&req, &status);
        printf("[%c] proc %d sent acc=%d to %d\n", dirmark, comm_rank, acc, next_comm_rank);
    }
}

int main(int argc, char** argv) {
    int n = 5;
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

    if (is_even(comm_rank)) {
        MPI_Barrier(MPI_COMM_WORLD);
        run_task_with_rank_filter(&is_even, FORWARD, comm_rank, comm_size, n);
    }

    if (is_odd(comm_rank)) {
        MPI_Barrier(MPI_COMM_WORLD);
        run_task_with_rank_filter(&is_odd, REVERSE, comm_rank, comm_size, n);
    }

    MPI_Finalize();
    return 0;
}
