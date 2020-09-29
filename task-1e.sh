#!/bin/bash
mpicc task-1e.c -o task-1e -lm -O3 -std=c11 &&
outopt=$1

if [[ "$outopt" == "runtime" ]]; then
    q=4096
    for nproc in 2 4 8 16; do
        for n in 4 256 16384; do
            sbatch -n $nproc -t 1 -p debug --wrap "mpiexec ./task-1e $outopt $n $q"
        done
    done
elif [[ "$outopt" == "stddev" ]]; then
    nproc=4
    for q in 16 256 4096 65536; do
        for n in 4 16 64 256 1024 4096 16384 65536; do
            sbatch -n $nproc -t 1 -p debug --wrap "mpiexec ./task-1e $outopt $n $q"
        done
    done
fi