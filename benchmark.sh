#!/bin/sh

ab -n 1000 -c 100 -g ab1.plt http://localhost:8080/
ab -n 1000 -c 100 -g ab2.plt http://localhost:8080/
ab -n 1000 -c 100 -g ab3.plt http://localhost:8080/

gnuplot benchmark-ab.plt

