#!/bin/bash
mpicc task-1d.c -o task-1d -lm -O3 -std=c11

nproc=4
exectask="mpiexec ./task-1d"
sbatch -n $nproc -t 1 -p debug --wrap "{ \
    $exectask 100; \
    $exectask 1000; \
    $exectask 10000; \
    $exectask 100000; \
}"