
# use line below for binary data demo
CFLAGS=-std=c99 -DBINARY_DATA
# use line below for ascii data demo
#CFLAGS=-std=c99

all: client server bi_client bi_server wu_client wu_server dosi_client analyze_adc to_plot

clean:
	rm -f *~ *.o client server bi_client bi_server wu_client wu_server dosi_client

analyze_adc.o: analyze_adc.c fft.h

fft.o: fft.c fft.h

analyze_adc: analyze_adc.o fft.o
	cc -o analyze_adc analyze_adc.o fft.o -lm -lfftw3
