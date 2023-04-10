//
// read a binary data file and convert to decimal with sample numbers
//

#include <stdio.h>
#include <stdint.h>

int main( int argc, char *argv[]) {
  FILE *fp;
  uint32_t s;
  int16_t v1, v2;
  int i = 0;
  
  fp = fopen( argv[1], "rb");
  while( fread( &s, sizeof(s), 1, fp)) {
    v1 = (int16_t)((s & 0xffff));
    v2 = (int16_t)((s>>16)&0xffff);
    printf("%d %d %d\n", i, v1, v2);
    // printf("%d %d\n", i, v1);
    ++i;
  }
}
