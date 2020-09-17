#!/bin/bash
mpicc task-1d.c -o task-1d -lm -O3 -std=c11

nproc=4
n=100
sbatch -n $nproc -t 1 -p debug --wrap "mpiexec ./task-1d $n"
n=1000
sbatch -n $nproc -t 1 -p debug --wrap "mpiexec ./task-1d $n"
n=10000
sbatch -n $nproc -t 1 -p debug --wrap "mpiexec ./task-1d $n"
n=100000
sbatch -n $nproc -t 1 -p debug --wrap "mpiexec ./task-1d $n"