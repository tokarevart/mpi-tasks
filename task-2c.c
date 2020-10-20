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
    bool wait_for_chain_final_sends = false;
    bool wait_for_send = false;
    MPI_Request* reqs = calloc(comm_size, sizeof(MPI_Request));
    MPI_Request* sendrecv_reqs = reqs + comm_size - 2;

    if (comm_rank < m) {
        int rnd = rand() % 101;
        if (n == 1) {
            printf("chain %d sum=%d", comm_rank, rnd);
            return;
        }

        Message msg = { comm_rank, 1, rnd };
        int send_to = rand() % comm_size;
        while (send_to == comm_rank) {
            send_to = rand() % comm_size;
        }
        MPI_Issend(&msg, sizeof(Message), MPI_BYTE, send_to, 0, MPI_COMM_WORLD, sendrecv_reqs);
        wait_for_send = true;
        printf(
            "proc %d sent     Message{chain_id=%d, acc_count=%d, acc=%d} to %d\n", 
            comm_rank, msg.chain_id, msg.acc_count, msg.acc, send_to
        );
    }
    
    int fin_chains_count = 0;
    while (fin_chains_count < m) {
        MPI_Request req;
        MPI_Status stats[2];

        Message msg;
        MPI_Irecv(&msg, sizeof(Message), MPI_BYTE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, sendrecv_reqs + 1);
        if (wait_for_chain_final_sends) {
            MPI_Waitall(comm_size, reqs, MPI_STATUSES_IGNORE);
            wait_for_chain_final_sends = false;
        } else if (wait_for_send) {
            MPI_Waitall(2, sendrecv_reqs, stats);
            wait_for_send = false;
            printf(
                "proc %d recieved Message{chain_id=%d, acc_count=%d, acc=%d} from %d\n", 
                comm_rank, msg.chain_id, msg.acc_count, msg.acc, stats[1].MPI_SOURCE
            );
        } 
        
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

            for (int i = 0; i < comm_rank; ++i) {
                MPI_Issend(&msg, sizeof(Message), MPI_BYTE, i, 0, MPI_COMM_WORLD, reqs + i);
            }
            for (int i = comm_rank + 1; i < comm_size; ++i) {
                MPI_Issend(&msg, sizeof(Message), MPI_BYTE, i, 0, MPI_COMM_WORLD, reqs + i - 1);
            }

            if (fin_chains_count == m) {
                MPI_Waitall(comm_size - 1, reqs, MPI_STATUSES_IGNORE);
                break;
            }
            wait_for_chain_final_sends = true;
            continue;
        }

        int send_to = rand() % comm_size;
        while (send_to == comm_rank) {
            send_to = rand() % comm_size;
        }
        MPI_Issend(&msg, sizeof(Message), MPI_BYTE, send_to, 0, MPI_COMM_WORLD, sendrecv_reqs);
        wait_for_send = true;
        printf(
            "proc %d sent     Message{chain_id=%d, acc_count=%d, acc=%d} to %d\n", 
            comm_rank, msg.chain_id, msg.acc_count, msg.acc, send_to
        );
    }

    free(reqs);
}

int main(int argc, char** argv) {
    int m = 2;
    int n = 2;

    int comm_rank, comm_size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &comm_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
    srand(comm_rank + 1);

    run_task(comm_rank, comm_size, m, n);

    MPI_Finalize();
    return 0;
}
