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
set style line 1 lt 3 lw 3 pt 3 lc "red"
set style line 2 lt 3 lw 3 pt 3 lc "green"
set style line 3 lt 3 lw 3 pt 3 lc "blue"

# plot data from bench1.tsv,bench2.tsv and bench3.tsv using column 10 with smooth sbezier lines
plot \
  "ab1.plt" using 10 smooth sbezier with lines title "Benchmark 1:", \
  "ab2.plt" using 10 smooth sbezier with lines title "Benchmark 2:", \
  "ab3.plt" using 10 smooth sbezier with lines title "Benchmark 3:"

