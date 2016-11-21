set terminal svg enhanced size 1000 800 fname "Times New Roman" fsize 26 solid
set encoding iso_8859_1

set border 4095 front linetype -1 linewidth 1.000
set ticslevel 0

set ylabel "MAC efficiency"  offset 0
set yrange [0:*]
set format y '%0.2f'


set xlabel "Maximum CW size / slots"
set xrange [2:*]

set grid xtics ytics back lw 1 lc rgb "#AFAFAF"
set key below


set output 'mac_efficiency_ca_short.svg'
plot "data.txt" using 3:(($1==42.24&&$2==2)?$4:1/0) with linespoints ls 1 lw 1 linecolor 2 pt 7 ps 0.3 title "n=2",\
"data.txt" using 3:(($1==42.24&&$2==3)?$4:1/0) with linespoints ls 1 lw 1 linecolor 3 pt 7 ps 0.3 title "n=3",\
"data.txt" using 3:(($1==42.24&&$2==4)?$4:1/0) with linespoints ls 1 lw 1 linecolor 4 pt 7 ps 0.3 title "n=4",\
"data.txt" using 3:(($1==42.24&&$2==5)?$4:1/0) with linespoints ls 1 lw 1 linecolor 1 pt 7 ps 0.3 title "n=5",\
"data.txt" using 3:(($1==42.24&&$2==6)?$4:1/0) with linespoints ls 1 lw 1 linecolor 5 pt 7 ps 0.3 title "n=6",\
"data.txt" using 3:(($1==42.24&&$2==7)?$4:1/0) with linespoints ls 1 lw 1 linecolor 6 pt 7 ps 0.3 title "n=7",\
"data.txt" using 3:(($1==42.24&&$2==8)?$4:1/0) with linespoints ls 1 lw 1 linecolor 7 pt 7 ps 0.3 title "n=8",\
"data.txt" using 3:(($1==42.24&&$2==9)?$4:1/0) with linespoints ls 1 lw 1 linecolor 8 pt 7 ps 0.3 title "n=9"
set output 'mac_efficiency_cd_short.svg'
plot "data.txt" using 3:(($1==42.24&&$2==2)?$5:1/0) with linespoints ls 1 lw 1 linecolor 2 pt 7 ps 0.3 title "n=2",\
"data.txt" using 3:(($1==42.24&&$2==3)?$5:1/0) with linespoints ls 1 lw 1 linecolor 3 pt 7 ps 0.3 title "n=3",\
"data.txt" using 3:(($1==42.24&&$2==4)?$5:1/0) with linespoints ls 1 lw 1 linecolor 4 pt 7 ps 0.3 title "n=4",\
"data.txt" using 3:(($1==42.24&&$2==5)?$5:1/0) with linespoints ls 1 lw 1 linecolor 1 pt 7 ps 0.3 title "n=5",\
"data.txt" using 3:(($1==42.24&&$2==6)?$5:1/0) with linespoints ls 1 lw 1 linecolor 5 pt 7 ps 0.3 title "n=6",\
"data.txt" using 3:(($1==42.24&&$2==7)?$5:1/0) with linespoints ls 1 lw 1 linecolor 6 pt 7 ps 0.3 title "n=7",\
"data.txt" using 3:(($1==42.24&&$2==8)?$5:1/0) with linespoints ls 1 lw 1 linecolor 7 pt 7 ps 0.3 title "n=8",\
"data.txt" using 3:(($1==42.24&&$2==9)?$5:1/0) with linespoints ls 1 lw 1 linecolor 8 pt 7 ps 0.3 title "n=9"
set output 'mac_efficiency_ca_middle.svg'
plot "data.txt" using 3:(($1==19873.9&&$2==2)?$4:1/0) with linespoints ls 1 lw 1 linecolor 2 pt 7 ps 0.3 title "n=2",\
"data.txt" using 3:(($1==19873.9&&$2==3)?$4:1/0) with linespoints ls 1 lw 1 linecolor 3 pt 7 ps 0.3 title "n=3",\
"data.txt" using 3:(($1==19873.9&&$2==4)?$4:1/0) with linespoints ls 1 lw 1 linecolor 4 pt 7 ps 0.3 title "n=4",\
"data.txt" using 3:(($1==19873.9&&$2==5)?$4:1/0) with linespoints ls 1 lw 1 linecolor 1 pt 7 ps 0.3 title "n=5",\
"data.txt" using 3:(($1==19873.9&&$2==6)?$4:1/0) with linespoints ls 1 lw 1 linecolor 5 pt 7 ps 0.3 title "n=6",\
"data.txt" using 3:(($1==19873.9&&$2==7)?$4:1/0) with linespoints ls 1 lw 1 linecolor 6 pt 7 ps 0.3 title "n=7",\
"data.txt" using 3:(($1==19873.9&&$2==8)?$4:1/0) with linespoints ls 1 lw 1 linecolor 7 pt 7 ps 0.3 title "n=8",\
"data.txt" using 3:(($1==19873.9&&$2==9)?$4:1/0) with linespoints ls 1 lw 1 linecolor 8 pt 7 ps 0.3 title "n=9"
set output 'mac_efficiency_cd_middle.svg'
plot "data.txt" using 3:(($1==19873.9&&$2==2)?$5:1/0) with linespoints ls 1 lw 1 linecolor 2 pt 7 ps 0.3 title "n=2",\
"data.txt" using 3:(($1==19873.9&&$2==3)?$5:1/0) with linespoints ls 1 lw 1 linecolor 3 pt 7 ps 0.3 title "n=3",\
"data.txt" using 3:(($1==19873.9&&$2==4)?$5:1/0) with linespoints ls 1 lw 1 linecolor 4 pt 7 ps 0.3 title "n=4",\
"data.txt" using 3:(($1==19873.9&&$2==5)?$5:1/0) with linespoints ls 1 lw 1 linecolor 1 pt 7 ps 0.3 title "n=5",\
"data.txt" using 3:(($1==19873.9&&$2==6)?$5:1/0) with linespoints ls 1 lw 1 linecolor 5 pt 7 ps 0.3 title "n=6",\
"data.txt" using 3:(($1==19873.9&&$2==7)?$5:1/0) with linespoints ls 1 lw 1 linecolor 6 pt 7 ps 0.3 title "n=7",\
"data.txt" using 3:(($1==19873.9&&$2==8)?$5:1/0) with linespoints ls 1 lw 1 linecolor 7 pt 7 ps 0.3 title "n=8",\
"data.txt" using 3:(($1==19873.9&&$2==9)?$5:1/0) with linespoints ls 1 lw 1 linecolor 8 pt 7 ps 0.3 title "n=9"
set output 'mac_efficiency_ca_long.svg'
plot "data.txt" using 3:(($1==39790.1&&$2==2)?$4:1/0) with linespoints ls 1 lw 1 linecolor 2 pt 7 ps 0.3 title "n=2",\
"data.txt" using 3:(($1==39790.1&&$2==3)?$4:1/0) with linespoints ls 1 lw 1 linecolor 3 pt 7 ps 0.3 title "n=3",\
"data.txt" using 3:(($1==39790.1&&$2==4)?$4:1/0) with linespoints ls 1 lw 1 linecolor 4 pt 7 ps 0.3 title "n=4",\
"data.txt" using 3:(($1==39790.1&&$2==5)?$4:1/0) with linespoints ls 1 lw 1 linecolor 1 pt 7 ps 0.3 title "n=5",\
"data.txt" using 3:(($1==39790.1&&$2==6)?$4:1/0) with linespoints ls 1 lw 1 linecolor 5 pt 7 ps 0.3 title "n=6",\
"data.txt" using 3:(($1==39790.1&&$2==7)?$4:1/0) with linespoints ls 1 lw 1 linecolor 6 pt 7 ps 0.3 title "n=7",\
"data.txt" using 3:(($1==39790.1&&$2==8)?$4:1/0) with linespoints ls 1 lw 1 linecolor 7 pt 7 ps 0.3 title "n=8",\
"data.txt" using 3:(($1==39790.1&&$2==9)?$4:1/0) with linespoints ls 1 lw 1 linecolor 8 pt 7 ps 0.3 title "n=9"
set output 'mac_efficiency_cd_long.svg'
plot "data.txt" using 3:(($1==39790.1&&$2==2)?$5:1/0) with linespoints ls 1 lw 1 linecolor 2 pt 7 ps 0.3 title "n=2",\
"data.txt" using 3:(($1==39790.1&&$2==3)?$5:1/0) with linespoints ls 1 lw 1 linecolor 3 pt 7 ps 0.3 title "n=3",\
"data.txt" using 3:(($1==39790.1&&$2==4)?$5:1/0) with linespoints ls 1 lw 1 linecolor 4 pt 7 ps 0.3 title "n=4",\
"data.txt" using 3:(($1==39790.1&&$2==5)?$5:1/0) with linespoints ls 1 lw 1 linecolor 1 pt 7 ps 0.3 title "n=5",\
"data.txt" using 3:(($1==39790.1&&$2==6)?$5:1/0) with linespoints ls 1 lw 1 linecolor 5 pt 7 ps 0.3 title "n=6",\
"data.txt" using 3:(($1==39790.1&&$2==7)?$5:1/0) with linespoints ls 1 lw 1 linecolor 6 pt 7 ps 0.3 title "n=7",\
"data.txt" using 3:(($1==39790.1&&$2==8)?$5:1/0) with linespoints ls 1 lw 1 linecolor 7 pt 7 ps 0.3 title "n=8",\
"data.txt" using 3:(($1==39790.1&&$2==9)?$5:1/0) with linespoints ls 1 lw 1 linecolor 8 pt 7 ps 0.3 title "n=9"
