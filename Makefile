
# use line below for binary data demo
CFLAGS=-std=c99 -DBINARY_DATA
# use line below for ascii data demo
#CFLAGS=-std=c99

all: client server bi_client bi_server wu_client wu_server dosi_client fftw_test

clean:
	rm -f *~ *.o client server bi_client bi_server wu_client wu_server dosi_client

fftw_test: fftw_test.o
	cc -o fftw_test fftw_test.o -lm -lfftw3
