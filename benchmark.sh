#!/bin/bash

set -e -u

ab_n=1000
ab_c=100

image=henet.git
debug_test=dist/Debug/GNU-Linux-x86/$image
release_test=dist/Release/GNU-Linux-x86/$image

function start_ab_benchmark()
{
    local name=$1

    sleep 1
    $name &
    sleep 1
}

function stop_ab_benchmark()
{
    local name=$1

    killall $name

    return 0
}

function run_ab_benchmark()
{
    local num=$1
    local con=$2
    local name=$3

    ab -n $num -c $con -g $name.plt http://localhost:8080/ > $name.txt
}

#
# Debug run
#
start_ab_benchmark $debug_test;
run_ab_benchmark   $ab_n, $ab_c, "ab-debug-1";
run_ab_benchmark   $ab_n, $ab_c, "ab-debug-2";
run_ab_benchmark   $ab_n, $ab_c, "ab-debug-3";
stop_ab_benchmark  $debug_test;

#
# Release run
#
start_ab_benchmark $release_test;
run_ab_benchmark   $ab_n, $ab_c, "ab-release-1";
run_ab_benchmark   $ab_n, $ab_c, "ab-release-2";
run_ab_benchmark   $ab_n, $ab_c, "ab-release-3";
stop_ab_benchmark  $release_test;

#
# Render benchmark with gnuplot
#
gnuplot benchmark-ab.plt

