#!/bin/bash
mpicc task-3b.c -o task-3b -lm -O3 -std=c11 &&
sbatch -n 2 -t 1 -p debug --wrap "mpiexec ./task-3b"