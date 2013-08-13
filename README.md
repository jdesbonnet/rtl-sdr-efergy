rtl-sdr-efergy
==============

Tool for use with RTL-SDR to decode radio frames from Efergy electricity
monitors. The Efergy Elite model is current supported and work is in progress
for the Efergy E2.

Use like this:

rtl_fm -f 433.55M -W -s 200000 -r 96000 - | ./elite-decode -r 96000


