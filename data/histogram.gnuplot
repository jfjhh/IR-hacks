#!/usr/bin/gnuplot
reset
unset key
set terminal pngcairo size 1024,768 font "Bitstream Vera Sans Mono,10"
set output "out.png"

set title "1e3 Raspberry Pi GPIO Pin Toggle Trials — Alex Striff"
set xlabel "Time Between Consecutive Pin 4 Toggles (s)"
set ylabel "Frequency"
set xrange [0.0000000596181:0.000000100]
set xtics border nomirror scale 59e-9 0,5e-9,114e-9 format "%0.2e"
set ytics border nomirror scale 0 0, 4, 100
set yrange [0:100]

Min=0
binwidth=100e-12
bin(x) = binwidth*(floor((x-Min)/binwidth)+0.5) + Min

set boxwidth binwidth
set style fill solid 0.42
set style line 1 linewidth 1 linecolor rgb "red"

#set datafile separator "\t"
plot "GPIO_nanoseconds.log" using (bin($1)):(1.0) smooth freq with boxes ls 1

