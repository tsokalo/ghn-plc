set terminal svg enhanced size 1000 800 fname "Times New Roman" fsize 26 solid
set encoding iso_8859_1

set border 4095 front linetype -1 linewidth 1.000
set ticslevel 0

set ylabel "Optimal maximum CW / slots"  offset 0
set yrange [0:*]
set format y '%0.2f'


set xlabel "Number of nodes"
set xrange [2:*]

set grid xtics ytics back lw 1 lc rgb "#AFAFAF"
set key below


set output 'mac_cw_opt_ca.svg'
plot "data_opt.txt" using 2:(($1==0.2)?$4:1/0) with linespoints ls 1 lw 1 linecolor 2 pt 7 ps 0.6 title "m=0.2",\
"data_opt.txt" using 2:(($1==0.4)?$4:1/0) with linespoints ls 1 lw 1 linecolor 1 pt 7 ps 0.6 title "m=0.4",\
"data_opt.txt" using 2:(($1==0.6)?$4:1/0) with linespoints ls 1 lw 1 linecolor 3 pt 7 ps 0.6 title "m=0.6",\
"data_opt.txt" using 2:(($1==0.8)?$4:1/0) with linespoints ls 1 lw 1 linecolor 8 pt 7 ps 0.6 title "m=0.8",\
"data_opt.txt" using 2:(($1==1)?$4:1/0) with linespoints ls 1 lw 1 linecolor 7 pt 7 ps 0.6 title "m=1.0"

set output 'mac_cw_opt_cd.svg'
plot "data_opt.txt" using 2:(($1==0.2)?$6:1/0) with linespoints ls 1 lw 1 linecolor 2 pt 7 ps 0.6 title "m=0.2",\
"data_opt.txt" using 2:(($1==0.4)?$6:1/0) with linespoints ls 1 lw 1 linecolor 1 pt 7 ps 0.6 title "m=0.4",\
"data_opt.txt" using 2:(($1==0.6)?$6:1/0) with linespoints ls 1 lw 1 linecolor 3 pt 7 ps 0.6 title "m=0.6",\
"data_opt.txt" using 2:(($1==0.8)?$6:1/0) with linespoints ls 1 lw 1 linecolor 8 pt 7 ps 0.6 title "m=0.8",\
"data_opt.txt" using 2:(($1==1)?$6:1/0) with linespoints ls 1 lw 1 linecolor 7 pt 7 ps 0.6 title "m=1.0"

set ylabel "Gain of MAC efficiency / %"  offset 0
set output 'mac_eff_gain.svg'
plot "data_opt.txt" using 2:(($1==0.2)?(($5-$3)/$3*100):1/0) with linespoints ls 1 lw 1 linecolor 2 pt 7 ps 0.6 title "m=0.2",\
"data_opt.txt" using 2:(($1==0.4)?(($5-$3)/$3*100):1/0) with linespoints ls 1 lw 1 linecolor 1 pt 7 ps 0.6 title "m=0.4",\
"data_opt.txt" using 2:(($1==0.6)?(($5-$3)/$3*100):1/0) with linespoints ls 1 lw 1 linecolor 3 pt 7 ps 0.6 title "m=0.6",\
"data_opt.txt" using 2:(($1==0.8)?(($5-$3)/$3*100):1/0) with linespoints ls 1 lw 1 linecolor 8 pt 7 ps 0.6 title "m=0.8",\
"data_opt.txt" using 2:(($1==1)?(($5-$3)/$3*100):1/0) with linespoints ls 1 lw 1 linecolor 7 pt 7 ps 0.6 title "m=1.0"
