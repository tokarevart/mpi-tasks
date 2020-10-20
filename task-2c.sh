#!/bin/bash
mpicc task-2c.c -o task-2c -lm -O3 -std=c11 &&
sbatch -n 2 -t 1 -p debug --wrap "mpiexec ./task-2c"