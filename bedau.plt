set terminal postscript color
set output "bedau.ps"
set data style line
set xlabel "Instructions Executed"
set title "Diversity"
plot "bedau.dat" using 1:2
set title "Mean Cum Activity"
plot "bedau.dat" using 1:3
set title "New Activity"
plot "bedau.dat" using 1:4
