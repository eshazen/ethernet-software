// analyze_adc.c - perform simple analysis on D-DOSI ADC data
//
// read a binary file containing one waveform capture
// for now expect the default 8192 samples
//
// command-line arguments:
//   -S              print power spectrum output
//   -P <npeaks>     scan for up to <npeaks> peaks in spectrum
//   -F <file>       specify input file
//   -A              ascii data (default is binary)
//   -V <var>        select variable (channel) 0 or 1
//   -N <npt>        process <npt> points in waveform
//   -I <ipt>        start processing at sample <ipt>
//   -O <ndds>       number of DDS outputs enabled
//   -L <slop>       specify threshold for peak finding.  Default is 1000
//                   This is in power spectrum units as a multiple of median power

#include <stdio.h>
#include <fftw3.h>
#include <stdlib.h>
#include <ctype.h>
#include <signal.h>
#include <math.h>
#include <stdint.h>

// file input buffer size
#define BUFSZ 80
//  
//  // ADC sampling rate
//  #define SAMP_RATE 250e6

#include "fft.h"

// check if there is an argument left in list
void check_arg( int i, int n, int argc, char *argv[]) {
  if( i+n > argc-1) {
    fprintf( stderr, "Expected value after -'%c'\n", argv[i][1]);
    exit(1);
  }
}

int main( int argc, char *argv[]) {

  int nPoints = 500;		/* waveform samples to process */
  int iPoint = 0;		/* initial point to process*/
  int wPoints = 0;		/* number of points read from file */
  int ascii = 0;		/* select text input */
  int debug = 0;
  int nDDS = 1;

  FILE *fp;
  double y0val, y1val;
  int xval;
  int k;
  char buff[BUFSZ];

  char* file_in = NULL;
  int varb = 0;
  int peaks = 0;
  int spec = 0;
  double peak_slope = 1000;

  double *in;

  if( argc > 1) {
    for( int i=1; i<argc; i++) {
      if( *argv[i] == '-') {
	switch( toupper( argv[i][1])) {
	case 'O':
	  check_arg( i, 1, argc, argv);
	  ++i;
	  nDDS = atoi(argv[i]);
	  break;
	case 'D':
	  debug++;
	  break;
	case 'S':
	  spec = 1;
	  break;
	case 'P':
	  check_arg( i, 1, argc, argv);
	  ++i;
	  peaks = atoi(argv[i]);
	  break;
	case 'F':
	  check_arg( i, 1, argc, argv);
	  ++i;
	  file_in = argv[i];
	  break;
	case 'V':
	  check_arg( i, 1, argc, argv);
	  ++i;
	  varb = atoi( argv[i]);
	  break;
	case 'N':
	  check_arg( i, 1, argc, argv);
	  ++i;
	  nPoints = atoi( argv[i]);
	  break;
	case 'I':
	  check_arg( i, 1, argc, argv);
	  ++i;
	  iPoint = atoi( argv[i]);
	  break;
	case 'L':
	  check_arg( i, 1, argc, argv);
	  ++i;
	  peak_slope = atof( argv[i]);
	  break;
	case 'A':
	  ascii = 1;
	  break;
	default:
	  printf("unknown option '%c'\n", argv[i][1]);
	  exit( 1);
	}
      }
    }
  } else {
    fprintf( stderr, "usage: %s [-f input_file] [-v var_no] [-n npoints]\n", argv[0]);
    fprintf( stderr, "          [-L peak_slope] [-s <no spectrum>] [-p max_peaks]\n");
    fprintf( stderr, "          [-I initial_point]\n");
    exit(1);
  }
  
  // --- text data input ---
  // read nPoints data points
  // expect data value in 2nd column
  if( ascii) {
    if( (fp = fopen( file_in, "r")) == NULL) {
      fprintf( stderr, "Can't open text input %s\n", file_in);
      exit(1);
    }

    // count number of lines
    wPoints = 0;
    while( fgets( buff, BUFSZ, fp))
      ++wPoints;
    fprintf( stderr, "%d lines in text file\n", wPoints);
    if( (in = calloc( wPoints, sizeof(*in))) == NULL) {
      fprintf( stderr, "Allocation fail for %d samples\n", wPoints);
      exit( -1);
    }
    rewind( fp);

    for( k=0; k<wPoints; k++) {
      if( fgets( buff, BUFSZ, fp) == NULL) {
	fprintf( stderr, "EOF on input after %d points\n", k);
	exit(1);
      }
      int ns = sscanf( buff, "%d %lf %lf", &xval, &y0val, &y1val);
      if( ns != 3) {
	fprintf( stderr, "unexpected data format in %s at line %d\n", buff, k);
	exit(1);
      }
      if( varb == 0)
	in[k] = y0val;
      else
	in[k] = y1val;
    }
  } else {
    // --- binary data input ---

    uint32_t s;
    int16_t v1, v2;

    if( (fp = fopen( file_in, "rb")) == NULL) {
      fprintf( stderr, "Can't open binary input %s\n", file_in);
      exit(1);
    }

    // read the file size in bytes
    fseek( fp, 0L, SEEK_END);
    long fsiz = ftell( fp);
    rewind( fp);
    fprintf( stderr, "Binary file has %ld bytes\n", fsiz);

    // find closest power of two
    for( int i=9; i<=16; i++) {	/* samples from 512...65536 */
      int bsiz = (1<<i) * sizeof(uint32_t); /* size in bytes */
      if( bsiz > fsiz) {
	wPoints = 1 << (i-1);
	break;
      }
    }
    int extra = (fsiz - (wPoints * sizeof(uint32_t))) / sizeof(uint32_t);
    fprintf( stderr, "Waveform has %d points with %d extra words\n", wPoints, extra);
    // number of waveform segments is number of extra words / 4
    if( extra % 4) {
      fprintf( stderr, "Unexpected number of extra words, not a multiple of 4!\n");
    }
    int numWf = extra / 4;
    fprintf( stderr, "Assuming there are %d waveform segments\n", numWf);

    if( (in = calloc( wPoints, sizeof(*in))) == NULL) {
      fprintf( stderr, "Allocation fail for %d samples\n", wPoints);
      exit( -1);
    }

    for( k=0; k<wPoints; k++) {
      if( !fread( &s, sizeof(s), 1, fp)) {
	fprintf(stderr, "EOF on binary read\n");
	exit(1);
      }
      v1 = (int16_t)((s & 0xffff));
      v2 = (int16_t)((s>>16)&0xffff);
      if( varb == 1)
	in[k] = v2;
      else
	in[k] = v1;
      if( debug) printf("%d raw 0x%08x v1 0x%04x v2 0x%04x in %f\n",
			k, s, v1, v2, in[k]);
    }
  }

  printf("DFT on %d points from %d to %d\n", nPoints, iPoint, iPoint+nPoints);
  if( iPoint+nPoints > wPoints) {
    fprintf( stderr, "invalid parameters\n");
    exit(-1);
  }

  if( debug) {
    for( int i=0; i<10; i++) printf("  %d = %f\n", i, in[iPoint+i]);
    printf( "   ...\n");
    for( int i=nPoints-10; i<nPoints; i++) printf("  %d = %f\n", i, in[iPoint+i]);
  }

  int nps = real_dft( nPoints, &in[iPoint], NULL, NULL);
  double power_spectrum[nps];
  double power_frequency[nps];

  nps = real_dft( nPoints, &in[iPoint], power_spectrum, power_frequency);
  if( nps < 0) {
    fprintf( stderr, "Error in DFT!\n");
    exit(-1);
  }

  if( spec)
    for (k = 1; k < nps; k++) {
      double mhz = power_frequency[k] / 1e6;
      printf("%8.2lf %lf\n", mhz, power_spectrum[k]);
    }


  if( peaks) {
    double peak_mag[peaks];
    double peak_freq[peaks];
    double median_power;
    int np = find_peaks( nps, power_spectrum, power_frequency, 
			 peaks, peak_mag, peak_freq,
			 peak_slope, &median_power);
    printf("%d peaks median power %9.3g\n", np, median_power);
    if( np) {
      for( int i=0; i<np; i++)
	printf("%d %9.3f MHz %9.3g mag\n", i, peak_freq[i]/1e6, peak_mag[i]);
    }
  }
}
