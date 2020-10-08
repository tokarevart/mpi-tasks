#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <mpi.h>

typedef struct {
    int chain_id;
    int acc_count;
    int acc;
} Message;

void run_task(int comm_rank, int comm_size, int m, int n) {
    if (comm_rank < m) {
        int rnd = rand() % 101;
        if (n == 1) {
            printf("chain %d sum=%d", comm_rank, rnd);
            return;
        }

        Message msg = { comm_rank, 1, rnd };
        int send_to = rand() % comm_size;
        MPI_Request req;
        MPI_Isend(&msg, sizeof(Message), MPI_BYTE, send_to, 0, MPI_COMM_WORLD, &req);
        MPI_Wait(&req, MPI_STATUS_IGNORE);
        printf(
            "proc %d sent     Message{chain_id=%d, acc_count=%d, acc=%d} to %d\n", 
            comm_rank, msg.chain_id, msg.acc_count, msg.acc, send_to
        );
    }
    
    int fin_chains_count = 0;
    while (fin_chains_count < m) {
        MPI_Request req;
        MPI_Status status;

        Message msg;
        MPI_Irecv(&msg, sizeof(Message), MPI_BYTE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &req);
        MPI_Wait(&req, &status);
        printf(
            "proc %d recieved Message{chain_id=%d, acc_count=%d, acc=%d} from %d\n", 
            comm_rank, msg.chain_id, msg.acc_count, msg.acc, status.MPI_SOURCE
        );
        if (msg.acc_count == n) {
            ++fin_chains_count;
            continue;
        }

        int rnd = rand() % 101;
        msg.acc += rnd;
        ++msg.acc_count;
        printf(
            "proc %d added %d to acc=%d\n", 
            comm_rank, rnd, msg.acc - rnd
        );
        if (msg.acc_count == n) {
            ++fin_chains_count;
            printf("chain %d sum=%d\n", msg.chain_id, msg.acc);

            MPI_Request* reqs = calloc(comm_size - 1, sizeof(MPI_Request));
            for (int i = 0; i < comm_rank; ++i) {
                MPI_Isend(&msg, sizeof(Message), MPI_BYTE, i, 0, MPI_COMM_WORLD, reqs + i);
            }
            for (int i = comm_rank + 1; i < comm_size; ++i) {
                MPI_Isend(&msg, sizeof(Message), MPI_BYTE, i, 0, MPI_COMM_WORLD, reqs + i - 1);
            }
            MPI_Waitall(comm_size - 1, reqs, MPI_STATUSES_IGNORE);
            free(reqs);
            continue;
        }

        int send_to = rand() % comm_size;
        MPI_Isend(&msg, sizeof(Message), MPI_BYTE, send_to, 0, MPI_COMM_WORLD, &req);
        MPI_Wait(&req, MPI_STATUS_IGNORE);
        printf(
            "proc %d sent     Message{chain_id=%d, acc_count=%d, acc=%d} to %d\n", 
            comm_rank, msg.chain_id, msg.acc_count, msg.acc, send_to
        );
    }
}

int main(int argc, char** argv) {
    int m = 2;
    int n = 3;

    int comm_rank, comm_size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &comm_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
    srand(comm_rank + 1);

    run_task(comm_rank, comm_size, m, n);

    MPI_Finalize();
    return 0;
}
