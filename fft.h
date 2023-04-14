//
// wrapper library to do simple real-only DFT
//   real_dft(...)     execute a real DFT
//   find_peaks(...)   look for peaks in the spectrum
//

// ADC sampling rate
#define SAMP_RATE 250e6

//
// execute the DFT.  Return -1 on error (nan or inf seen) or size of power_spectrum
// (call with power_spectrum=NULL to just get required size)
//
// fill in power_spectrum with relative power and power_freq with frequency in Hz
//
int real_dft( int nPoints, double *data, double *power_spectrum, double *power_freq);

//
// find peaks in power spectrum
// input parameters:
//   spec_size                   - number of points in spectrum
//   power_spectrum, power_freq  - power spectrum, frequencies
//   max_peak                    - maximum number of peaks
//   peak_ratio                  - slope threshold for peak
//                                 (slope vs median magnitude)
// output parameters
//   peak_mag, peak_freq         - list of peaks
//   median_power                - median of power spectrum
//   function return             - number of peaks found
//
int find_peaks( int spec_size, double *power_spectrum, double *power_freq,
		int max_peak, double *peak_mag, double *peak_freq,
		double peak_ratio, double *median_power);

// find median in a list of doubles
double find_median( int np, double* data);
