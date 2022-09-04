#include <stdio.h>
#include <errno.h>
#include <err.h>
#include <termios.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>

void print_usage(char * const argv[])
{
  fprintf(stderr, "Usage: %s [-v] [-h]\n  -v  verbose output\n", argv[0]);
}

int main(int argc, char *argv[])
{
  int verbose = false;
  int opt;
  while ((opt = getopt(argc, argv, "vh")) != -1) {
    switch (opt) {
      case 'v':
        verbose = true;
        break;;
      default: /* ? */ 
        print_usage(argv);
        exit(opt == 'h' ? EXIT_SUCCESS : EXIT_FAILURE);
    }
  }

  FILE *tty = fopen("/dev/tty", "r+");
  if (tty == NULL) {
    err(errno, "Opening TTY failed");
  }
  struct termios old_termios;
  unsigned int r,g,b;
  int start, stop_r, stop_g, stop_b;
  
  /* Save terminal state and make it raw */
  if (tcgetattr(fileno(tty), &old_termios)) err(errno, "tcgetattr");
  struct termios raw_termios = old_termios;
  cfmakeraw(&raw_termios);
  if (tcsetattr(fileno(tty), TCSAFLUSH, &raw_termios)) err(errno, "tcsetattr (raw)");
  
  int Ps = 11;
  fprintf(tty, "\033]%d;?\033\\", Ps);
  int ret = fscanf(tty, "\033]%*d;%*[rgb]%n:%x%n/%x%n/%x%n\033\\", &start, &r, &stop_r, &g, &stop_g, &b, &stop_b);

  /* Restore terminal state */
  if (tcsetattr(fileno(tty), TCSAFLUSH, &old_termios)) err(errno, "tcsetattr (restore)");
  if (ret == EOF || ret != 3) err(errno, "terminal color format not recognized");

  /* Calculate scale factor for each rgb value */
  int rmax = (1 << (4 * (stop_r - start - 1))) - 1;
  int gmax = (1 << (4 * (stop_g - stop_r - 1))) - 1;
  int bmax = (1 << (4 * (stop_b - stop_g - 1))) - 1;
  /* CCIR 601 luminance value */
  double luminance = (0.299 * (double)r/(double)rmax) + (0.587 * (double)g/(double)gmax) + (0.114 * (double)b/(double)bmax);
  if (verbose)
  {
    printf("(r, g, b): (%0*x, %0*x, %0*x)\nLuminance: %f\n", (stop_r - start - 1), r, (stop_g - stop_r - 1), g, (stop_b - stop_g - 1), b, luminance);
  } else {
    printf("%f\n", luminance);
  }
}

