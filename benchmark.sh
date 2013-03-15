#!/bin/bash

set -e -u

ab_n=1000
ab_c=100

image=henet.git
debug_test=dist/Debug/GNU-Linux-x86/$image
release_test=dist/Release/GNU-Linux-x86/$image

sleep 1

#
# Debug run
#

sleep 1

$debug_test &

sleep 3

ab -n $ab_n -c $ab_c -g ab-debug-1.plt http://localhost:8080/ > ab-debug-1.txt
ab -n $ab_n -c $ab_c -g ab-debug-2.plt http://localhost:8080/ > ab-debug-2.txt
ab -n $ab_n -c $ab_c -g ab-debug-3.plt http://localhost:8080/ > ab-debug-3.txt

sleep 3

killall $image

#
# Release run
#

sleep 1

$release_test &

sleep 3

ab -n $ab_n -c $ab_c -g ab-release-1.plt http://localhost:8080/ > ab-release-1.txt
ab -n $ab_n -c $ab_c -g ab-release-2.plt http://localhost:8080/ > ab-release-2.txt
ab -n $ab_n -c $ab_c -g ab-release-3.plt http://localhost:8080/ > ab-release-3.txt

sleep 3

killall $image

#
# Render benchmark with gnuplot
#

gnuplot benchmark-ab.plt

