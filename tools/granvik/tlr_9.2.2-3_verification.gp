set terminal postscript color enhanced 10
set out
unset key
set size 1,1
set origin 0,0
set multiplot
# Upper panel
set size 1,0.5
set origin 0.0,0.5
set xlabel "UT [MJD]"
set ylabel "Nr of new objects discovered /\nNr of visible objects"
plot '~/tmp/tlr_922_3_verification_plot.dat' using ($1):($2/$3) with lines
# Lower left
set size 0.5,0.5
set origin 0.0,0.0
set xlabel "UT [MJD]"
set ylabel "Nr of new objects discovered"
plot '~/tmp/tlr_922_3_verification_plot.dat' using 1:2 with lines
# Lower right
set size 0.5,0.5
set origin 0.5,0.0
set xlabel "UT [MJD]"
set ylabel "Nr of visible objects"
plot '~/tmp/tlr_922_3_verification_plot.dat' using 1:3 with lines
unset multiplot
reset
