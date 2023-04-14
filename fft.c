//
// DFT functions
// (see fft.h for docs)
//

#include <stdio.h>
#include <fftw3.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <string.h>

#include "fft.h"

// #define DEBUG

// file input buffer size
#define BUFSZ 80

//
// run a DFT on real data if pwrSpec != NULL
// check for invalid results, return -1 if so
// return size of pwrSpec
//
int real_dft( int nPoints, double *data, double *power_spectrum, double *power_freq)
{
  fftw_complex* out;
  fftw_plan p;
  int k;

  int spwr = (nPoints+1)/2;	/* size of power spectrum */

  if( power_spectrum == NULL)
    return spwr;

  double interval = SAMP_RATE / nPoints; /* output frequency interval */
  fprintf( stderr, "For sample rate %f/s using %d points, interval = %g Hz\n",
	   SAMP_RATE, nPoints, interval);

  if( (out = calloc( nPoints, sizeof( fftw_complex))) == NULL) {
    fprintf( stderr, "Can't allocate memory for DFT\n");
    exit(-1);
  }

  p = fftw_plan_dft_r2c_1d( nPoints, data, out, FFTW_ESTIMATE);
  fftw_execute( p);

  // check for errors
  for( k=0; k<nPoints; k++)
    if( !isfinite(out[k][0]) || !isfinite(out[k][1])) {
      fprintf( stderr, "ERROR in output (infinity) at %d\n", k);
      return -1;
    }

  power_spectrum[0] = out[0][0]*out[0][0];  /* DC component */

  for (k = 1; k < spwr; ++k) {  /* (k < N/2 rounded up) */
    double freq = interval * k;
    power_spectrum[k] = out[k][0]*out[k][0] + out[nPoints-k][0]*out[nPoints-k][0];
    power_freq[k] = freq;
  }
  if (nPoints % 2 == 0) /* N is even */
    power_spectrum[nPoints/2] = out[nPoints/2][0]*out[nPoints/2][0];  /* Nyquist freq. */

  fftw_destroy_plan(p);

  return spwr;
}

//
// find peaks
//
#define PEAK_RANGE 5

int find_peaks( int spec_size, double *power_spectrum, double *power_freq,
		int max_peak, double *peak_mag, double *peak_freq, double peak_ratio,
		double *median_power)
{
  double diff[PEAK_RANGE-1];
  int x0 = (PEAK_RANGE-1)/2;	/* start looking here */
  int x1 = spec_size-x0;	/* stop looking here */
  int peaks = 0;
  
  // find median value
  *median_power = find_median( spec_size, power_spectrum);
#ifdef DEBUG
  printf("median = %g\n", *median_power);
#endif  
  for( int k=x0; k<=x1; k++) {
    // calculate derivatives
    for( int i=0; i<PEAK_RANGE-1; i++) {
      diff[i] = power_spectrum[k+i+1] /(*median_power)-power_spectrum[k+i] /(*median_power);
    }
// #ifdef DEBUG
//     printf("k=%d freq=%f diffs( ", k, power_freq[k]/1e6);
//     for( int j=0; j<PEAK_RANGE-1; j++)
//       printf(" %d=%f", j, diff[j]);
//     printf("\n");
// #endif    
    if( diff[1] > peak_ratio && diff[2] < -peak_ratio) {
      
      if( peaks < max_peak) {
#ifdef DEBUG      
	printf("PEAK %d at %d (%8.2f val=%f)  (", peaks, k+2, power_freq[k+2],
	       power_spectrum[k+2]/median_power);
	for( int j=0; j<PEAK_RANGE-1; j++)
	  printf(" %d=%f", j, diff[j]);
	printf(")\n");
	
#endif
	peak_mag[peaks] = power_spectrum[k+2];
	peak_freq[peaks] = power_freq[k+2];
	++peaks;
      }
    }
  }
  return peaks;
}

int cfun( const void * a, const void *b) {
  return( *(double*)a - *(double*)b);
}

//
// find median in a list of doubles
//
double find_median( int np, double* data)
{
  int npt = np;
  if( (np % 2))
    ++npt;
  
  double sorted[npt];
  memcpy( sorted, data, np * sizeof(double));

  qsort( sorted, np, sizeof(double), cfun);

  // return the value of the middle
  return sorted[np/2];
}
