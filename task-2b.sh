#!/bin/bash
mpicc task-2b.c -o task-2b -lm -O3 -std=c11 &&
sbatch -n 4 -t 1 -p debug --wrap "mpiexec ./task-2b"