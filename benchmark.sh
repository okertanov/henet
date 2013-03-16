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
gnuplot << _EO_PLOT_

#
# Benchmark template using Apache ab and GNU plot
#
# see http://blog.secaserver.com/2012/03/web-server-benchmarking-apache-benchmark/
#

#
# Legend for plt file:
#       ctime: Connection Time
#       dtime: Processing Time
#       ttime: Total Time
#       wait: Waiting Time
#

# output as png image
set terminal png size 800,600

# save file to "benchmark.png"
set output "benchmark.png"

# aspect ratio for image size
set size 1,1

# graph title
set title "Benchmark for henet. `date`"

# enable grid on y-axis
# set grid y

# x-axis label
set xlabel "Request"

# y-axis label
set ylabel "Response Time (ms)"

# styles
unset border
set grid back

set style line 1 lt 1 lw 1 linecolor rgb "#A00000"
set style line 2 lt 1 lw 1 linecolor rgb "#B00000"
set style line 3 lt 1 lw 1 linecolor rgb "#C00000"

set style line 4 lt 1 lw 1 linecolor rgb "#00A000"
set style line 5 lt 1 lw 1 linecolor rgb "#00B000"
set style line 6 lt 1 lw 1 linecolor rgb "#00C000"

# plot data from bench1.tsv,bench2.tsv and bench3.tsv using column 10 with smooth sbezier lines
plot \
    "ab-debug-1.plt"   using 10 smooth sbezier with lines ls 1 title "ab -n $ab_n -c $ab_c debug 1   ", \
    "ab-debug-2.plt"   using 10 smooth sbezier with lines ls 2 title "ab -n $ab_n -c $ab_c debug 2   ", \
    "ab-debug-3.plt"   using 10 smooth sbezier with lines ls 3 title "ab -n $ab_n -c $ab_c debug 3   ", \
    "ab-release-1.plt" using 10 smooth sbezier with lines ls 4 title "ab -n $ab_n -c $ab_c release 1 ", \
    "ab-release-2.plt" using 10 smooth sbezier with lines ls 5 title "ab -n $ab_n -c $ab_c release 2 ", \
    "ab-release-3.plt" using 10 smooth sbezier with lines ls 6 title "ab -n $ab_n -c $ab_c release 3 "

_EO_PLOT_

