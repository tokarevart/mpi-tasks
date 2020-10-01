#!/bin/bash
mpicc task-2a.c -o task-2a -lm -O3 -std=c11 &&
sbatch -n 3 -t 1 -p debug --wrap "mpiexec ./task-2a"