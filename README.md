rtl-sdr-efergy
==============

Tool for use with RTL-SDR (a Linux driver for Realtek RTL2832U radio dongles)
to decode radio frames from Efergy electricity monitors. The Efergy Elite model
is current supported and work is in progress for the Efergy E2.

Current release is 0.1 (13 Aug 2013). It's alpha grade right now, but works
reasonably well (for me anyhow).

Use like this:

rtl_fm -f 433.55M -W -s 200000 -r 96000 - | ./elite-decode -r 96000


TODO:

Many tools that use RTL-SDR link directly to the shareable library. It would
neater to have just one binary rather than relying in pipes.


More information:

http://jdesbonnet.blogspot.ie/2010/09/smart-electricity-meter-based-on-efergy.html
http://jdesbonnet.blogspot.ie/2011/04/smart-electricity-meter-based-on-efergy.html
http://electrohome.pbworks.com/w/page/34379858/Efergy-Elite-Wireless-Meter-Hack

