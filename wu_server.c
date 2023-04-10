/*
 * File:	server.c
 * Description:	Simple TCP server after Peterson and Davie
 * 
 * Wu's version starts sending data immediately after connection
 */

#define SEND_BINARY

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>

#define SERVER_PORT	5432
// #define MAX_PENDING	5

// #define SERVER_PORT    5791
#define MAX_PENDING     1
#define MAX_LINE	256

typedef enum { false = 0, true = 1 } bool;

int
main(int argc, char *argv[])
{
  struct sockaddr_in sin;
  char buf[MAX_LINE];
  char reply[MAX_LINE];
  int conn, line;
  int len;
  int s, new_s;
  int c_one = 1;
  int rc;

  /*
   * Initialize the addres data structure
   */
  memset((void *)&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_port = htons(SERVER_PORT);

  /*
   * Create a socket
   */
  if ((s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
    fprintf(stderr, "%s: socket - %s\n", argv[0], strerror(errno));
    exit(1);
  }

  /* set the "reuse" option */
  rc = setsockopt( s, SOL_SOCKET, SO_REUSEADDR, &c_one, sizeof(c_one));

  /*
   * Bind an address to the socket
   */
  if (bind(s, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
    fprintf(stderr, "%s: bind - %s\n", argv[0], strerror(errno));
    exit(1);
  }

  /*
   * Set the length of the listen queue
   */
  if (listen(s, MAX_PENDING) < 0) {
    fprintf(stderr, "%s: listen - %s\n", argv[0], strerror(errno));
    exit(1);
  }

  conn = 0;
	
  /*
   * Loop accepting new connections and servicing them
   */
  while (true) {
    len = sizeof(sin);
    if ((new_s = accept(s, (struct sockaddr *)&sin, &len)) < 0) {
      fprintf(stderr, "%s: accept - %s\n",
	      argv[0], strerror(errno));
      exit(1);
    }
    printf("New connection\n");
    ++conn;
    line = 0;

    while ( line < 10) {
      ++line;
#ifdef BINARY_DATA
      for( int i=0; i<MAX_LINE; i++)
	reply[i] = i ^ 0xff;
      len = MAX_LINE;
#else
      sprintf( reply, "Connection %d Line %d\n", conn, line);
      len = strlen( reply);
#endif
      send( new_s, reply, len, 0);
    }

    close(new_s);
  }
}
/*
 * End server.c
 */
