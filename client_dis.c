/*
 * File:	client.c
 * Description:	Simple TCP/IP client from Peterson and Davie
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define SERVER_PORT	5432
#define MAX_LINE	256

int
main(int argc, char *argv[])
{
  FILE *fp;
  struct hostent *hp;
  struct sockaddr_in sin;
  char *host;
  char buf[MAX_LINE];
  int s;
  int len;

  if (argc == 2) {
    host = argv[1];
  } else {
    fprintf(stderr, "usage: %s host\n", argv[0]);
    exit(1);
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
  memcpy((void *)&sin.sin_addr, hp->h_addr, hp->h_length);
  sin.sin_port = htons(SERVER_PORT);
	
  
  while (fgets(buf, sizeof(buf), stdin) != NULL) {
    buf[MAX_LINE - 1] = 0;
    len = strlen(buf);

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

    send(s, buf, len, 0);

    close( s);
  }

}

/*
 * End client.c
 */
