
set datafile separator ","

set title 'Phase plot of Sys1'
set ylabel 'State x[1]'
set xlabel 'State x[0]'
set grid
#set term png
#set output '<Output file name>.png'
set xrange [-1.5:1.5]
set yrange [-1.5:1.5]
plot '../bin/log.csv' using 2:3 with lines title "x[0], x[1]"