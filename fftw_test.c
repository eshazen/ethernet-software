
#include <stdio.h>
#include <fftw3.h>
#include <stdlib.h>
#include <ctype.h>
#include <signal.h>
#include <math.h>

// file input buffer size
#define BUFSZ 80

// ADC sampling rate
#define SAMP_RATE 250e6

// check if there is an argument left in list
void check_arg( int i, int n, int argc, char *argv[]) {
  if( i+n > argc-1) {
    fprintf( stderr, "Expected value after -'%c'\n", argv[i][1]);
    exit(1);
  }
}

int main( int argc, char *argv[]) {

  int nPoints = 500;		/* arbitrary! */

  fftw_plan p;
  int k;

  FILE *fp;
  char buff[BUFSZ];
  double y0val, y1val;
  int xval;

  char* file_in = NULL;
  int varb = 1;

  if( argc > 1) {
    for( int i=1; i<argc; i++) {
      if( *argv[i] == '-') {
	switch( toupper( argv[i][1])) {
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
	default:
	  printf("unknown option '%c'\n", argv[i][1]);
	  exit( 1);
	}
      }
    }
  } else {
    fprintf( stderr, "usage: %s [-f input_file] [-v var_no] [-n npoints]\n", argv[0]);
    exit(1);
  }
  
  double in[nPoints];
  fftw_complex out[nPoints];
  double power_spectrum[nPoints/2+1];

  double interval = SAMP_RATE / nPoints; /* output frequency interval */
  fprintf( stderr, "For sample rate %f/s using %d points, interval = %g Hz\n",
	   SAMP_RATE, nPoints, interval);

  if( (fp = fopen( file_in, "r")) == NULL) {
    fprintf( stderr, "Can't open input %s\n", file_in);
    exit(1);
  }

  // read nPoints data points
  // expect data value in 2nd column
  for( k=0; k<nPoints; k++) {
    if( fgets( buff, BUFSZ, fp) == NULL) {
      fprintf( stderr, "EOF on input after %d points\n", k);
      exit(1);
    }
    int ns = sscanf( buff, "%d %lf %lf", &xval, &y0val, &y1val);
    if( ns != 3) {
      fprintf( stderr, "unexpected data format in %s at line %d\n", buff, k);
      exit(1);
    }
    if( varb == 1)
      in[k] = y0val;
    else
      in[k] = y1val;
  }

  p = fftw_plan_dft_r2c_1d( nPoints, in, out, FFTW_ESTIMATE);
 
  fftw_execute( p);

  // check for errors
  for( k=0; k<nPoints; k++)
    if( !isfinite(out[k][0]) || !isfinite(out[k][1])) {
      fprintf( stderr, "ERROR in output (infinity) at %d\n", k);
      exit(-1);
    }

  power_spectrum[0] = out[0][0]*out[0][0];  /* DC component */

  for (k = 1; k < (nPoints+1)/2; ++k) {  /* (k < N/2 rounded up) */
    double freq = interval * k;
    double mhz = freq / 1e6;
    power_spectrum[k] = out[k][0]*out[k][0] + out[nPoints-k][0]*out[nPoints-k][0];
    printf("%8.2lf %lf %lf\n", mhz, out[k][0], power_spectrum[k]);
  }
  if (nPoints % 2 == 0) /* N is even */
    power_spectrum[nPoints/2] = out[nPoints/2][0]*out[nPoints/2][0];  /* Nyquist freq. */

  

  fftw_destroy_plan(p);
}
