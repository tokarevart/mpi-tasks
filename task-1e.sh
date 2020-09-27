#!/bin/bash
mpicc task-1e.c -o task-1e -lm -O3 -std=c11 &&
sbatch -n 2 -t 1 -p debug --wrap "mpiexec ./task-1e 4 16"