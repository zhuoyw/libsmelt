#!/bin/bash

echo "compile and generate rank files"
rm -r rank_files
mkdir rank_files
./compile.sh

CPUFILE=/proc/cpuinfo

NUMLOG=$(lscpu | grep "^CPU(s)" | tr -d '[[:space:]]' | cut -d ":" -f 2)

if [ $NUMLOG > $NUMPHY ]
then
    echo "Hyperthreads enabled"
    ./parse_cores 1
else
    echo "Hyperthreads disabled"
    ./parse_cores
fi

/home/haeckir/openmpi-1.10.2/bin/mpirun -H localhost -n $NUMLOG -rf rank_files/rfile_fill_$NUMLOG -mca rmaps_rank_file_physical 1 -mca --bind-to core -mca blt sm,self broadcast 1

/home/haeckir/openmpi-1.10.2/bin/mpirun -H localhost -n $NUMLOG -rf rank_files/rfile_fill_$NUMLOG -mca rmaps_rank_file_physical 1 -mca --bind-to core -mca blt sm,self broadcast 

rm -r rank_files
