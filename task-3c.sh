#!/bin/bash
mpicc task-3c.c -o task-3c -lm -O3 -std=c11 &&
sbatch -n 3 -t 1 -p debug --wrap "mpiexec ./task-3c"