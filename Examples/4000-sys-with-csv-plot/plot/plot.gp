
set datafile separator ","

set title 'Plot of Sys1'
set ylabel 'State and Output'
set xlabel 'Time'
set grid
#set term png
#set output '<Output file name>.png'
plot '../bin/log.csv' using 1:2 with lines title "x[0]", \
     '../bin/log.csv' using 1:3 with lines title "x[1]", \
     '../bin/log.csv' using 1:4 with lines title "x[2]", \
     '../bin/log.csv' using 1:5 with lines title "y[0]", \
     '../bin/log.csv' using 1:6 with lines title "u[0]"