#!/usr/bin/env bash
# used to test the speed of the murmur has function, at various block sizes and thread counts
for th in 1 2 3 4 5 6 7; do
    echo "THREADS: $th"
    for bs in 2048 8192 32768 131072 524288; do
        ./test_murmur_speed_vs_thread $th $bs
    done
done