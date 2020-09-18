#!/bin/bash
mpicc task-1d.c -o task-1d -lm -O3 -std=c11

for nproc in 2 4 8 16
do
    for n in 100 1000 10000 100000
    do
        sbatch -n $nproc -t 1 -p debug --wrap "mpiexec ./task-1d $n"
    done
done
