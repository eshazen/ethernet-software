/*
 * dosi_client.c - simple client for testing using the DOSI server
 * 
 * usage:
 *   ./dosi_client [-c config_file] [-d data_file] [-h host]
 * 
 *   config_file is a list of text commands to be sent.  Usually ends in 'g' to run a sweep
 *   data_file is a binary data file to store the data
 *   host is the IP address of the host
 *
 *   after 'g' is sent, write raw binary data to the data file and exit
 */

// dump data in binary
#define BIN_DUMP

#include <arpa/inet.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#define SERVER_PORT	27015
#define MAX_LINE	1000000

// check if there is an argument left in list
void check_arg( int i, int n, int argc, char *argv[]) {
  if( i+n > argc-1) {
    fprintf( stderr, "Expected value after -'%c'\n", argv[i][1]);
    exit(1);
  }
}

int main(int argc, char *argv[])
{
  FILE *fp = NULL;
  FILE *fpc = NULL;
  struct hostent *hp;
  struct sockaddr_in sin;
  char *host;
  char *conf;
  char *dataf;
  char buf[MAX_LINE];
  int s;
  int len;
  int go = 0;

  host = "169.254.3.3";
  conf = NULL;
  dataf = NULL;

  if( argc > 1) {
    for( int i=1; i<argc; i++) {
      if( *argv[i] == '-') {
	switch( toupper( argv[i][1])) {
	case 'H':
	  check_arg( i, 1, argc, argv);
	  ++i;
	  host = argv[i];
	  break;
	case 'C':
	  check_arg( i, 1, argc, argv);
	  ++i;
	  conf = argv[i];
	  break;
	case 'D':
	  check_arg( i, 1, argc, argv);
	  ++i;
	  dataf = argv[i];
	  break;
	default:
	  printf("unknown option '%c'\n", argv[i][1]);
	  exit( 1);
	}
      }
    }
  }

  printf("host = %s  config = %s   data file = %s\n", host, conf, dataf);

  if( dataf) {
    if( (fp = fopen( dataf, "wb")) == NULL) {
      fprintf( stderr, "error opening output file %s\n", dataf);
      exit(1);
    }
  }

  /* open config file */
  if( conf) {
    if( (fpc = fopen( conf, "r")) == NULL) {
      fprintf( stderr, "error opening config file %s\n", conf);
      exit(1);
    }
  }

  /*
   * Translate host name into peer's IP address
   */
  if ((hp = gethostbyname(host)) == NULL) {
    fprintf(stderr, "%s: unknown host %s\n", argv[0], argv[1]);
    exit(1);
  }

  /*
   * Initialize the address data structure
   */
  memset((void *)&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  memcpy((void *)&sin.sin_addr, hp->h_addr_list[0], hp->h_length);
  sin.sin_port = htons(SERVER_PORT);
	
  /*
   * Create a socket
   */
  if ((s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
    perror("socket");
    exit(1);
  }

  /*
   * Create a connection between the socket and the server
   */
  if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
    perror("connect");
    exit(1);
  }

  /*
   * send config commands
   */
  if( conf) {
    while( !go && (fgets( buf, sizeof(buf), fpc) != NULL) ) {
      if( *buf == 'g') {
	printf("Saw GO\n");
	go = 1;
      } else {
	buf[MAX_LINE - 1] = 0;
	len = strlen(buf);
	send(s, buf, len, 0);
	printf("Sent %s", buf);
      }
    }
  }

  while (go || fgets(buf, sizeof(buf), stdin) != NULL) {
    buf[MAX_LINE - 1] = 0;
    len = strlen(buf);
    send(s, buf, len, 0);
    if( go || buf[0] == 'g') {
      printf("Expecting data\n");
      sleep(1);
      if( (len = recv( s, buf, sizeof(buf), 0))) {
	printf("recv %d bytes", len);
	if( len >= MAX_LINE) {
	  len = MAX_LINE - 1;
	}
	buf[len] = 0;
	if( fp) {
	  fwrite( buf, sizeof(buf[0]), len, fp);
	  printf("Wrote %d bytes to %s\n", len, dataf);
	} else {
	  for( int i=0; i<len; i++) {
	    printf("%6d: 0x%02x", i, buf[i]);
	    if( buf[i] >= 0x20 && buf[i] < 0x7f)
	      printf(" '%c'", buf[i]);
	    printf("\n");
	  }
	}
      }
      fclose( fp);
      exit(0);
    }
  }
}

/*
 * End client.c
 */
