#!/usr/bin/gnuplot
reset
unset key
set terminal pngcairo size 1024,768 font "Bitstream Vera Sans Mono,10"
set output "out.png"

set title "36e3 Raspberry Pi GPIO Pin Toggle Trials (High Pri.) — Alex Striff"
set xlabel "Time Between Consecutive Pin 4 Toggles (s)"
set ylabel "Frequency"
#set xrange [0.0000000596181:0.000000100]
set xtics border nomirror format "%0.2e"
set ytics border
#set yrange [0:100]

Min=0
binwidth=10e-12
bin(x) = binwidth*(floor((x-Min)/binwidth)+0.5) + Min

set boxwidth binwidth
set style fill solid 0.42
set style line 1 linewidth 1 linecolor rgb "red"

set label " " textcolor rgb "blue" point lt 3 offset 0,0 at 60.3234e-9,0
set label " " textcolor rgb "blue" point lt 3 offset 0,0 at 60.3234e-9,250
set label " " textcolor rgb "blue" point lt 3 offset 0,0 at 60.3234e-9,500
set label " " textcolor rgb "blue" point lt 3 offset 0,0 at 60.3234e-9,750
set label " " textcolor rgb "blue" point lt 3 offset 0,0 at 60.3234e-9,1000
set label " " textcolor rgb "blue" point lt 3 offset 0,0 at 60.3234e-9,1250
set label " " textcolor rgb "blue" point lt 3 offset 0,0 at 60.3234e-9,1500
set label " " textcolor rgb "blue" point lt 3 offset 0,0 at 60.3234e-9,1750
set label " " textcolor rgb "blue" point lt 3 offset 0,0 at 60.3234e-9,2000
set label " " textcolor rgb "blue" point lt 3 offset 0,0 at 60.3234e-9,2250
set label " " textcolor rgb "blue" point lt 3 offset 0,0 at 60.3234e-9,2500
set label " " textcolor rgb "blue" point lt 3 offset 0,0 at 60.3234e-9,2750
set label " " textcolor rgb "blue" point lt 3 offset 0,0 at 60.3234e-9,3000
set label " " textcolor rgb "blue" point lt 3 offset 0,0 at 60.3234e-9,3250
set label " " textcolor rgb "blue" point lt 3 offset 0,0 at 60.3234e-9,3500
set label " " textcolor rgb "blue" point lt 3 offset 0,0 at 60.3234e-9,3750
set label " " textcolor rgb "blue" point lt 3 offset 0,0 at 60.3234e-9,4000
set label " Mean" textcolor rgb "blue" point lt 3 offset 0,0 at 60.3234e-9,4250
set label " " textcolor rgb "blue" point lt 3 offset 0,0 at 60.3234e-9,4500
set label " " textcolor rgb "blue" point lt 3 offset 0,0 at 60.3234e-9,4750
set label " " textcolor rgb "blue" point lt 3 offset 0,0 at 60.3234e-9,5000
set label " " textcolor rgb "blue" point lt 3 offset 0,0 at 60.3234e-9,5250
set label " " textcolor rgb "blue" point lt 3 offset 0,0 at 60.3234e-9,5500
set label " " textcolor rgb "blue" point lt 3 offset 0,0 at 60.3234e-9,5750
set label " " textcolor rgb "blue" point lt 3 offset 0,0 at 60.3234e-9,6000
set label " " textcolor rgb "blue" point lt 3 offset 0,0 at 60.3234e-9,6250
set label " " textcolor rgb "blue" point lt 3 offset 0,0 at 60.3234e-9,6500
set label " " textcolor rgb "blue" point lt 3 offset 0,0 at 60.3234e-9,6750
set label " " textcolor rgb "blue" point lt 3 offset 0,0 at 60.3234e-9,7000
set label " " textcolor rgb "blue" point lt 3 offset 0,0 at 60.3234e-9,7250
set label " " textcolor rgb "blue" point lt 3 offset 0,0 at 60.3234e-9,7500
set label " " textcolor rgb "blue" point lt 3 offset 0,0 at 60.3234e-9,7750
set label " " textcolor rgb "blue" point lt 3 offset 0,0 at 60.3234e-9,8000
set label " " textcolor rgb "blue" point lt 3 offset 0,0 at 60.3234e-9,8250
set label " " textcolor rgb "blue" point lt 3 offset 0,0 at 60.3234e-9,8500
set label " " textcolor rgb "blue" point lt 3 offset 0,0 at 60.3234e-9,8750
set label " " textcolor rgb "blue" point lt 3 offset 0,0 at 60.3234e-9,9000

#set datafile separator "\t"
plot "GPIO_nanoseconds-highpri.log" using (bin($1)):(1.0) smooth freq with boxes ls 1

