#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <math.h>

static void fhdl( int sig, siginfo_t *sip, ucontext_t* scp) {
  printf("got sig = %d\n", sig);
}

int main() {
  struct sigaction act;

  act.sa_sigaction = (void(*))fhdl;
  sigemptyset( &act.sa_mask);
  act.sa_flags = SA_SIGINFO;

  if( sigaction( SIGFPE, &act, (struct sigaction *)0) != 0) {
    perror("Yikes in sigaction()");
    exit( -1);
  }

  double x = 1.0;
  double y = 0.0;
  double z = x / y;

  printf( "%lf %lf %lf\n", x, y, z);

  if( !isfinite(z))
    perror("Divide error!");
}
