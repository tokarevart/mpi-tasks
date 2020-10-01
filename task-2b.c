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

int to_odd(int idx) {
    return idx * 2 + 1;
}

int to_even(int idx) {
    return idx * 2;
}

int to_idx(int odd) {
    return odd / 2;
}

bool is_odd(int idx) {
    return idx % 2 == 1;
}

bool is_even(int idx) {
    return idx % 2 == 0;
}

typedef int (*IdxToRank)(int);

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

void run_task_with_mapper(IdxToRank torank, Direction dir, int comm_rank, int comm_size, int n) {
    if (torank(to_idx(comm_rank)) != comm_rank) {
        printf("run_task_with_mapper fn got wrong params\n");
        return;
    }

    char dirmark;
    if (dir == FORWARD) {
        dirmark = 'F';
    } else {
        dirmark = 'R';
    }
    comm_rank = to_idx(comm_rank);
    // yeah... it is magic, but i couldn't find a better way to compute the new comm_size
    comm_size = to_idx(comm_size * 2 - min_int(torank(to_idx(comm_size)), comm_size));

    int prev_comm_rank;
    int next_comm_rank;
    if (dir == FORWARD) {
        prev_comm_rank = circular_dec(comm_rank, comm_size);
        next_comm_rank = circular_inc(comm_rank, comm_size);
    } else {
        prev_comm_rank = circular_inc(comm_rank, comm_size);
        next_comm_rank = circular_dec(comm_rank, comm_size);
    }
    
    int acc = 0;
    for (int i = 0;; ++i) {
        int count = i * comm_size + comm_rank;
        if (count >= n) {
            break;
        }

        MPI_Request req;
        MPI_Status status;
        if (count > 0) {
            MPI_Irecv(&acc, 1, MPI_INT, torank(prev_comm_rank), MPI_ANY_TAG, MPI_COMM_WORLD, &req);
            MPI_Wait(&req, &status);
            printf("[%c] proc %d received acc=%d from %d\n", dirmark, torank(comm_rank), acc, torank(prev_comm_rank));
        }

        int rnd = rand() % 101;
        acc += rnd;
        printf("[%c] proc %d added %d to acc=%d\n", dirmark, torank(comm_rank), rnd, acc - rnd);

        if (count == n - 1) {
            printf("[%c] sum=%d\n", dirmark, acc);
            break;
        }

        MPI_Isend(&acc, 1, MPI_INT, torank(next_comm_rank), 0, MPI_COMM_WORLD, &req);
        MPI_Wait(&req, &status);
        printf("[%c] proc %d sent acc=%d to %d\n", dirmark, torank(comm_rank), acc, torank(next_comm_rank));
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
        run_task_with_mapper(&to_even, FORWARD, comm_rank, comm_size, n);
    }

    if (is_odd(comm_rank)) {
        MPI_Barrier(MPI_COMM_WORLD);
        run_task_with_mapper(&to_odd, REVERSE, comm_rank, comm_size, n);
    }

    MPI_Finalize();
    return 0;
}
