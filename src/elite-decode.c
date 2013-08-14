/**
 * Efergy Elite decoder. Uses rtl_fm tool to capture radio transmissions.
 * (NB: this is not for the Efergy E2 which uses a different PHY/protocol)
 *
 * This version can read directly from rtl_fm output.
 * rtl_fm -f 433.55M -W -s 200000 -r 96000 - | ./elite-decode -r 96000
 *
 * Copyright (c) 2013, Joe Desbonnet, jdesbonnet@gmail.com
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 * * Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * * The name of the contributors may not be used to endorse or promote
 *   products derived from this software without specific prior written
 *   permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Release History:
 *
 * Version 0.1, (13 Aug 2013)
 * First public release. Very much 'alpha' quality.
 *
 */


#include <stdio.h>
#include <stdint.h>
#include <sys/time.h>

#define SAMPLE_RATE (96000)
#define SAMPLE_SCALE (SAMPLE_RATE/48000)

/**
 * Globals used with get_next_sample().
 * TODO: use static instead?
 */
int t = 0;
int f = 0;
int ntpt = 0;
int py=0;

/**
 * Read next sample from rtl_fm (signed 16bit integer, little endian).
 * This uses zero-crossing technique to extract frequency information
 * from the data stream.
 *
 * Update global t, f (length, in samples, of positive half cycle)
 * The reason I am using just the positive half-cycle is that unless
 * the waveform is exactly centered you'll get different values for
 * the positive and negative half cycle. 
 *
 * I also tried the obvious full cycle length but found I didn't get
 * a clean separation of frequencies at frequency transition points.
 * I might have been measuring 180 degrees out of phase: averaging 
 * the second half cycle of one frequency with the first half cycle 
 * of the other frequency. I think this is worth exploring further.
 */
int get_next_sample () {

	int16_t y;

	t++;
	y = fgetc(stdin) | fgetc(stdin)<<8;
	if (py < 0 && y >= 0) {
		ntpt = t;
	} else if (py >= 0 && y < 0) {
		f = t-ntpt;
	}
	py = y;
	return f;
}


int main (int argc, char**argv) {
	int i=0;
	int v=0,pv=0;
	int bit_count = 0;
	int shift_reg = 0;
	int symbol = 0;

	int lc=0; // run length counter

	int freq=0;
	int prevfreq=0;
	int prevfreqt=0;
	int nbit=0;
	float nbitf=0;

	int tstart;

	uint8_t frame[16];
	// Start of frame also available as 32bit integer for quick good frame test
	uint32_t *sof = (int *)(&frame);
	int byte_count=0;

	int milliamps;

	// Loop forever
	while(!feof(stdin)) {


		//
		// Look for start of frame.
		//
		lc = 0;
		while ( !feof(stdin) ) {
			v = get_next_sample();
			if (v == pv) {
				lc++;
			} else {
				if (lc > (SAMPLE_RATE/16) ) {
					break;
				} else {
					lc = 0;
				}
				pv = v;
			}

			// display activity		
			i++;
			if (i % 10000 == 0) {
				fprintf (stderr,"*");
				fflush (stderr);
			}
		}

		fprintf (stderr,"\nSTART OF FRAME: lc=%d ts=%d ",lc,t);

	
		//
		// Read frame. From previous analysis 
		// ( http://jdesbonnet.blogspot.ie/2010/09/smart-electricity-meter-based-on-efergy.html ) 
		// each symbol is 2ms in duration.
		// logic 0: 3 cycles of 1500Hz
		// logic 1: 4 cycles of 2000Hz
		//

		bit_count = 0;
		byte_count = 0;
		shift_reg = 0;
		lc=0;
		tstart=t;


		while ( !feof(stdin) ) {

			
			// logic 0: 1500Hz = 64samples @ 96ksps
			// logic 1: 2000Hz = 48 samples @ 96ksps

			// Duration of positive half cycle, so /2.
			// ie ~ 32 and ~24
			v = get_next_sample();
		

			// TODO: make sample rate independent. Currently hard coded to 96ksps

			//if ( v>=((13*SAMPLE_RATE)/48000)  &&  v <= ((17*SAMPLE_RATE)/48000) ) {
			if ( v>=32  &&  v <= 34 ) {
				freq=0;
			} else if ( v >= 24  &&  v <= 27 ) {
				freq=1;
			}

			if (freq == prevfreq) {
				lc++;
			} else {
				// Attempting here to make it sample rate independent
				nbit = (lc+(45*SAMPLE_RATE)/48000)/((90*SAMPLE_RATE)/48000);
				nbitf = (float)lc/90.0;
				//fprintf (stdout, "%d (%d %d %d %f)\n", prevfreq, lc, nbit, t-prevfreqt, nbitf);
				//fflush (stdout);
				while (nbit !=0) {
					shift_reg <<= 1;
					shift_reg |= prevfreq;
					bit_count++;
					if (bit_count%8==0) {
						fprintf (stderr,"[%02x] ",shift_reg & 0xff);
						frame[byte_count++] = shift_reg & 0xff;
					}
					nbit--;
				}
				lc=0;
				prevfreq=freq;
				prevfreqt=t;

				if (bit_count >= 8*12) {
					fprintf (stderr,"te=%d dur=%d bc=%d\n", t, t-tstart,bit_count);
					break;
				}
			}
	
		}

		// TODO: CRC byte at end still missing.

		// Discard bad frames. Check start of frame for hex values: AB AB AB 2D
		fprintf (stderr,"sof=%x ", *sof);
		if (*sof == 0x2dababab) {
			// End of Frame
			milliamps = (frame[8] | (frame[7]&0x0f)<<8)*10;
			fprintf (stderr, "mA=%d\n", milliamps);

			fprintf (stdout, "%d %d\n", time(NULL), milliamps);
			fflush (stdout);
		} else {
			fprintf (stderr, "bad frame\n");
		}

	} // loop forever

}

