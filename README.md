rtl-sdr-efergy
==============

Tool for use with RTL-SDR (a Linux driver for Realtek RTL2832U radio dongles)
to decode radio frames from Efergy electricity monitors. The Efergy Elite model
is current supported and work is in progress for the Efergy E2.

Current release is 0.1 (13 Aug 2013). It's alpha grade right now, but works
reasonably well (for me anyhow).

Use like this:

rtl_fm -f 433.55M -W -s 200000 -r 96000 - | ./elite-decode -r 96000 > electricity.dat

Debug info will go to the screen, and power use (current in mA) will be channeled
to stdout. You can plot in gnuplot with the following gnuplot script:

```
set title "Electricity use chart"
set grid
set xlabel "Time"
set ylabel "Power (kW)"
set xdata time          # X axis is time
set timefmt "%s"     # Input file time format is unix epoc time
set format x "%R"   # Display time in 24 hour notation on the X axis
set terminal png
set output 'electricity.png'
plot 'electricity.dat' using 1:($2)*230/1e6 title 'Electricity' with lines
```


TODO:

Many tools that use RTL-SDR link directly to the shareable library. It would
neater to have just one binary rather than relying in pipes.


More information:

http://jdesbonnet.blogspot.ie/2010/09/smart-electricity-meter-based-on-efergy.html
http://jdesbonnet.blogspot.ie/2011/04/smart-electricity-meter-based-on-efergy.html
http://electrohome.pbworks.com/w/page/34379858/Efergy-Elite-Wireless-Meter-Hack

