//
// read a binary data file and convert to decimal with sample numbers
//

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

int main( int argc, char *argv[]) {
  FILE *fp;
  uint32_t s;
  int16_t v[2];
  int i = 0;
  char *fin;

  int var = 1;

  if( argc > 1) {
    var = atoi( argv[1]);
    fin = argv[2];
  } else
    fin = argv[1];

  fp = fopen( fin, "rb");
  while( fread( &s, sizeof(s), 1, fp)) {
    v[0] = (int16_t)((s & 0xffff));
    v[1] = (int16_t)((s>>16)&0xffff);
    if( var < 3)
      printf("%d %d\n", i, v[var]);
    else
      printf("%d %d %d\n", i, v[0], v[1]);
    ++i;
  }
}
