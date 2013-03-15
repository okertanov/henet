#
# Benchmark template using Apache ab and GNU plot
#
# see http://blog.secaserver.com/2012/03/web-server-benchmarking-apache-benchmark/
#

#
# ab -n 1000 -c 100 http://localhost:8080/
#
# Legend for plt file:
#       ctime: Connection Time
#       dtime: Processing Time
#       ttime: Total Time
#       wait: Waiting Time
#

# output as png image
set terminal png

# save file to "benchmark.png"
set output "benchmark.png"

# graph title
set title "Benchmark for henet. `date`"

# aspect ratio for image size
set size 1,1

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
  "ab-debug-1.plt"   using 10 smooth sbezier with lines ls 1 title "Debug 1:",   \
  "ab-debug-2.plt"   using 10 smooth sbezier with lines ls 2 title "Debug 2:",   \
  "ab-debug-3.plt"   using 10 smooth sbezier with lines ls 3 title "Debug 3:",   \
  "ab-release-1.plt" using 10 smooth sbezier with lines ls 4 title "Release 1:", \
  "ab-release-2.plt" using 10 smooth sbezier with lines ls 5 title "Release 2:", \
  "ab-release-3.plt" using 10 smooth sbezier with lines ls 6 title "Release 3:"

