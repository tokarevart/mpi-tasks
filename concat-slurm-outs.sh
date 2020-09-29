#!/bin/bash
outsdirname=slurm-outs

minid_init=100000000
minid=$minid_init
maxid=0
for file in slurm-*.out; do
    numout=${file#"slurm-"}
    num=${numout%".out"}
    if [[ "$num" != *"-"* ]]; then
        (( num < minid )) && minid=$num
        (( num > maxid )) && maxid=$num
    fi
done
output=slurm-$minid-$maxid.out

(( minid != minid_init && maxid != 0 )) && : > $output
for file in slurm-*.out; do
    numout=${file#"slurm-"}
    if [[ "$numout" != *"-"* ]]; then
        cat $file >> $output
    fi
done

mkdir -p $outsdirname
for file in slurm-*.out; do
    numout=${file#"slurm-"}
    if [[ "$numout" != *"-"* ]]; then
        mv $file ./$outsdirname/
    fi
done
