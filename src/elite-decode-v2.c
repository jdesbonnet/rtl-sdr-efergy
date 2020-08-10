/**
 * Efergy Elite decoder. Uses rtl_fm tool to capture radio transmissions.
 * (NB: this is not for the Efergy E2 which uses a different PHY/protocol)
 *
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
 * 
 * This version can read directly from rtl_fm output.
 * rtl_fm -f 433.55M -W -s 200000 -r 96000 - | ./elite-decode -r 96000
 * Release History:
 *
 * Version 0.2, (9 Aug 2020)
 * Version 0.1, (13 Aug 2013)
 *
 */


#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>

#define SAMPLE_RATE (96000)
#define SAMPLE_SCALE (SAMPLE_RATE/48000)

#define SYNC (0xAB2D)

int calc_crc (uint64_t data) {
	int i,sum=0;
	uint8_t *c;
	c = (uint8_t *)&data;
	for (i = 0; i < 8; i++) {
		sum += *c;
		c++;
	}
	return sum&0xff;
}

void output_data (uint64_t data) {

}


int main (int argc, char**argv) {

	int i=0,n;
	uint16_t sample;
	uint16_t frame_sync=0;
	int lpf=0;
	int lpf2=0;
	int a=0;

	int s0,s1;
	int zero_cross_t1;
	int period;
	int freq=2000, prev_freq=1500, run_count=0;
	int symbol;
	int last_symbol_t = 0;
	int current;

	uint64_t frame_data;
	int frame_bit;
	int frame_crc;
	int crc = 0;    // in CRC bits of frame
	int crc_fail_count=0;

	int debug = 0;

	while(!feof(stdin)) {

		// Read next sample: note this is a FM frequency (not raw signal amplitude)
		n = fread(&sample, 2, 1, stdin);
		if ( n != 1) {
			fprintf(stderr,"error reading data n=%d\n",n);
			break;
		}

		// smothing filter: is this necessary?
		lpf += sample;
		lpf -= lpf/4;

		s1 = lpf/4;

		// used to determine 'DC' component
		lpf2 += s1;
		lpf2 -= lpf2/64;

		// remove DC
		s1 -= lpf2/64;

		// amplitude 
		a += (s1 > 0 ? s1: -s1);
		a -= a/256;

		if ((a/256)>2048) {
			continue;
		}

		
		// logic 0: 1500Hz = 64samples @ 96ksps
		// logic 1: 2000Hz = 48 samples @ 96ksps

		//if ( (s1 > 0) && (s0 <= 0) ) {
		if ( (s1 < 0) && (s0 >= 0) ) {
			// + to - crossing

			period = i - zero_cross_t1;
			zero_cross_t1 = i;

			if ( (period >= 40) && (period < 54) ) {
				freq = 2000;
			} else if ( (period >= 54) && (period < 64) ) {
				freq = 1500;
			} else if ( period > 100 && crc == 1 && frame_bit == 7) {
				// CRC messy because last bit of CRC does not cross immediately
				freq = prev_freq;
				if (debug>1) {
					fprintf(stderr,"CRC");
				}
			} else {
				freq = -1;
			}

			if (freq == prev_freq) {
				run_count++;
			} else {
				run_count=1;
			} 

			symbol = -1;
			if ( (freq == 1500) && (run_count == 3) ) {
				symbol = 0;
			}
			if ( (freq == 2000) && (run_count == 4) ) {
				symbol = 1;
			}

			if (symbol >= 0) {
				if (debug>1) {
					fprintf (stderr,"%d",symbol);
					fflush (stderr);
				}
				if (debug>1  && (i - last_symbol_t > 10000) ) {
					fprintf (stderr,"\n");
				}
				run_count = 0;
				last_symbol_t = i;
				frame_sync <<= 1;
				frame_data <<= 1;
				if (symbol == 1) {
					frame_sync |= 1;
					frame_data |= 1;
				}
				frame_bit++;
				if (frame_sync == SYNC ) {
					if (debug>1) {
						fprintf (stderr, "SYNC");
					}
					frame_data=0;
					frame_bit=0;
				}

				if (crc == 1 && frame_bit == 8) {
					if (debug>1) {
						fprintf (stderr,"crc=%d,%d ",(int)(frame_data&0xff), frame_crc);
					}
					
					//fprintf(stdout,"%d %d %d\n", (unsigned)time(NULL), current,  (int)(frame_data&0xff) == frame_crc ? 1 : 0);
					if ((int)(frame_data&0xff) == frame_crc) {
						fprintf(stdout,"%d %d %d\n", (unsigned)time(NULL), current,  crc_fail_count);
					} else {
						crc_fail_count++;
					}
					fflush(stdout);
					
					crc = 0;
				}

			}

			if (frame_bit==64) {
				frame_crc = calc_crc(frame_data);
				current =  (int)((frame_data>>24)&0xfff);
				//fprintf(stderr,"%d %d\n", (unsigned)time(NULL), current);
				//fflush(stderr);
				if (debug>1) {
					fprintf(stderr,"D");
				}
				frame_bit=0;
				crc = 1;
			}



			if (freq == -1) {
				run_count = 0;
			}

			prev_freq = freq;

		}

		if (debug>1) {
			fprintf (stdout,"DEBUG %d %d %d %d %d %d %d\n", i, sample, s1, lpf2/64, (period<0 ? 0 : period*100), run_count*1000,frame_bit*100 );
		}

		s0 = s1;
		i++;

	}



}

