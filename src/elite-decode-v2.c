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
#include <sys/time.h>

#define SAMPLE_RATE (96000)
#define SAMPLE_SCALE (SAMPLE_RATE/48000)



int main (int argc, char**argv) {

	int i=0,n;
	uint16_t sample;
	int lpf=0;
	int lpf2=0;
	int lpf3=0;

	int s0,s1;
	int zero_cross_t0, zero_cross_t1;
	int period;
	int freq, prev_freq, run_count=0;

	while(!feof(stdin)) {

		n = fread(&sample, 2, 1, stdin);
		if ( n != 1) {
			fprintf(stderr,"error reading data n=%d\n",n);
			break;
		}

		lpf2 += sample;
		lpf2 -= lpf2/256;

		// Remove 'DC' offset
		//sample -= lpf2/256;

		lpf += sample;
		lpf -= lpf/8;

		lpf3 += lpf/8;
		lpf3 -= lpf3/32;

		fprintf (stdout,"%d %d %d %d %d\n", i, sample, lpf/8, (lpf/8)-51500 , lpf3/32);

		s1 = lpf/8 - 51500;

		if ( (s1 > 0) && (s0 < 0) ) {
			zero_cross_t1 = i;
			period = zero_cross_t1 - zero_cross_t0;


			// logic 0: 1500Hz = 64samples @ 96ksps
			// logic 1: 2000Hz = 48 samples @ 96ksps

			if ( (period >= 60) && (period < 68) ) {
				freq = 1500;
			} else if ( (period >= 44) && (period < 55) ) {
				freq = 2000;
			} else {
				freq = -1;
			}

			if (freq == prev_freq) {
				run_count++;
			}

			if ( (freq == 1500) && (run_count == 3) ) {
				fprintf(stderr,"0");
				run_count = 0;
			}
			if ( (freq == 2000) && (run_count == 4) ) {
				fprintf(stderr,"1");
				run_count = 0;
			}
			if (freq == -1) {
				run_count = 0;
			}

			prev_freq = freq;
			

			//fprintf(stderr,"period=%d freq=%d rc=%d\n",period, freq, run_count);

			//fprintf(stderr,"period=%d freq=%d rc=%d\n",period, freq, run_count);

			zero_cross_t0 = zero_cross_t1;
		}

		s0 = s1;
		i++;

	}



}

