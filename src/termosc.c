#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <err.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <ctype.h>

#define TIMEOUT 50 // deci-seconds (0.1s)

// C0 set
#define NUL "\x00"
#define SOH "\x01"
#define STX "\x02"
#define ETX "\x03"
#define EOT "\x04"
#define ENQ "\x05"
#define ACK "\x06"
#define BEL "\x07"
#define BS  "\x08"
#define HT  "\x09"
#define LF  "\x0a"
#define VT  "\x0b"
#define FF  "\x0c"
#define CR  "\x0d"
#define SO  "\x0e"
#define LS1 SO
#define SI  "\x0f"
#define LS0 SI
#define DLE "\x10"
#define DC1 "\x11"
#define DC2 "\x12"
#define DC3 "\x13"
#define DC4 "\x14"
#define NAK "\x15"
#define SYN "\x16"
#define ETB "\x17"
#define CAN "\x18"
#define EM  "\x19"
#define SUB "\x1a"
#define ESC "\x1b"
#define IS4 "\x1c"
#define IS3 "\x1d"
#define IS2 "\x1e"
#define IS1  "\x1f"

// C1 set
//      --  ESC "\x40"
//      --  ESC "\x41"
#define BPH ESC "\x42"
#define NBH ESC "\x43"
//      --  ESC "\x44"
#define NEL ESC "\x45"
#define SSA ESC "\x46"
#define ESA ESC "\x47"
#define HTS ESC "\x48"
#define HTJ ESC "\x49"
#define VTS ESC "\x4a"
#define PLD ESC "\x4b"
#define PLU ESC "\x4c"
#define RI  ESC "\x4d"
#define SS2 ESC "\x4e"
#define SS3 ESC "\x4f"
#define DCS ESC "\x50"
#define PU1 ESC "\x51"
#define PU2 ESC "\x52"
#define STS ESC "\x53"
#define CCH ESC "\x54"
#define MW  ESC "\x55"
#define SPA ESC "\x56"
#define EPA ESC "\x57"
#define SOS ESC "\x58"
//      --  ESC "\x59"
#define SCI ESC "\x5a"
#define CSI ESC "\x5b"
#define ST  ESC "\x5c"
#define OSC ESC "\x5d"
#define PM  ESC "\x5e"
#define APC  ESC "\x5f"

// CONTROL SEQUENCES
#define ICH  "\x40"
#define CUU  "\x41"
#define CUD  "\x42"
#define CUF  "\x43"
#define CUB  "\x44"
#define CNL  "\x45"
#define CPL  "\x46"
#define CHA  "\x47"
#define CUP  "\x48"
#define CHT  "\x49"
#define ED   "\x4a"
#define EL   "\x4b"
#define IL   "\x4c"
#define DL   "\x4d"
#define EF   "\x4e"
#define EA   "\x4f"
#define DCH  "\x50"
#define SSE  "\x51"
#define CPR  "\x52"
#define SU   "\x53"
#define SD   "\x54"
#define NP   "\x55"
#define PP   "\x56"
#define CTC  "\x57"
#define ECH  "\x58"
#define CVT  "\x59"
#define CBT  "\x5a"
#define SRS  "\x5b"
#define PTX  "\x5c"
#define SDS  "\x5d"
#define SIMD "\x5e"
//      --   "\x5f"
#define HPA  "\x60"
#define HPR  "\x61"
#define REP  "\x62"
#define DA   "\x63"
#define VPA  "\x64"
#define VPR  "\x65"
#define HVP  "\x66"
#define TBC  "\x67"
#define SM   "\x68"
#define MC   "\x69"
#define HPB  "\x6a"
#define VPB  "\x6b"
#define RM   "\x6c"
#define SGR  "\x6d"
#define DSR  "\x6e"
#define DAQ  "\x6f"

#define SL   "\x20\x40"
#define SR   "\x20\x41"
#define GSM  "\x20\x42"
#define GSS  "\x20\x43"
#define FNT  "\x20\x44"
#define TSS  "\x20\x45"
#define JFY  "\x20\x46"
#define SPI  "\x20\x47"
#define QUAD "\x20\x48"
#define SSU  "\x20\x49"
#define PFS  "\x20\x4a"
#define SHS  "\x20\x4b"
#define SVS  "\x20\x4c"
#define IGS  "\x20\x4d"
//      --   "\x20\x4e"
#define IDCS "\x20\x4f"
#define PPA  "\x20\x50"
#define PPR  "\x20\x51"
#define PPB  "\x20\x52"
#define SPD  "\x20\x53"
#define DTA  "\x20\x54"
#define SHL  "\x20\x55"
#define SLL  "\x20\x56"
#define FNK  "\x20\x57"
#define SPQR "\x20\x58"
#define SEF  "\x20\x59"
#define PEC  "\x20\x5a"
#define SSW  "\x20\x5b"
#define SACS "\x20\x5c"
#define SAPV "\x20\x5d"
#define STAB "\x20\x5e"
#define GCC  "\x20\x5f"
#define TATE "\x20\x60"
#define TALE "\x20\x61"
#define TAC  "\x20\x62"
#define TCC  "\x20\x63"
#define TSR  "\x20\x64"
#define SCO  "\x20\x65"
#define SRCS "\x20\x66"
#define SCS  "\x20\x67"
#define SLS  "\x20\x68"
//      --   "\x20\x69"
//      --   "\x20\x6a"
#define SCP  "\x20\x6b"
//      --   "\x20\x6c"
//      --   "\x20\x6d"
//      --   "\x20\x6e"
//      --   "\x20\x6f"

#define IS_P(P) (P >= 0x30 && P <= 0x3F)
#define IS_I(I) (I >= 0x20 && I <= 0x2F)
#define IS_F(F) (F >= 0x40 && I <= 0x7D)


static struct termios old_termios = {0};
static int tty = -1;
char *buf = NULL;
size_t buflen = 0;

static void restore_tty(void)
{
  for (size_t i = 0; i < buflen; ++i) {
    if (ioctl(tty, TIOCSTI, &buf[i]) == -1) err(errno, "ioctl");
  }
  tcsetattr(tty, TCSANOW, &old_termios);
}

int fillbuf(void)
{
  static size_t bufsz = 4096;
  if (!buf || 2 * buflen >= bufsz) {
    buf = realloc(buf, sizeof *buf * bufsz);
    bufsz *= 2;
  }
  int nr = read(tty, &buf[buflen], bufsz - buflen);
  if (nr < 0) err(errno, "Read failed");
  buflen += nr;
  return nr;
}

int main(void)
{
  // Open the terminal
  tty = open("/dev/tty", O_RDWR);
  if (tty < 0) err(errno, "Opening TTY failed");
  
  /* Save terminal state and make it raw */
  if (tcgetattr(tty, &old_termios)) err(errno, "tcgetattr");
  atexit(restore_tty);

  struct termios raw_termios = old_termios;
  cfmakeraw(&raw_termios);
  raw_termios.c_cc[VTIME] = TIMEOUT; 
  raw_termios.c_cc[VMIN] = 0;
  if (tcsetattr(tty, TCSADRAIN, &raw_termios)) err(errno, "tcsetattr (raw)");
 
  
  size_t i = 0;
  size_t end = 0;
  
  // Send the OSC "background color" query, and a standard ECMA-48 Device Status Report request after it
  {
    const char seq[] = OSC "11;?" ST CSI "5" DSR;
    write(tty, seq, sizeof seq - 1);
  }
  
  for (int success=false;;) {
    int nr = fillbuf();
    if (nr == 0) err(errno=ENOSYS, "Terminal is not ECMA-48 compliant... it's been 30 years");

    // Scan through the buffer for control sequences
    for (size_t i = 0; i < buflen; ++i) {
      // Check for response to OSC
      if (i + sizeof OSC "11;rgb:" - 1 <= buflen &&
          memcmp(OSC "11;rgb:", &buf[i], sizeof OSC "11;rgb:" - 1) == 0
         ) {
        size_t cname_begin = i + sizeof OSC "11;rgb:" - 1;
        for (size_t j = cname_begin + 5; j <= cname_begin + 14 && j < buflen; ++j)
        {
          if (j + sizeof ST - 1 <= buflen &&
              memcmp(ST, &buf[j], sizeof ST - 1) == 0) {
            char *cname = malloc(sizeof *cname * (j - cname_begin + 1));
            memcpy(cname, &buf[cname_begin], j - cname_begin);
            cname[j - cname_begin] = '\0';
            unsigned int red = 0, green = 0, blue = 0;
            int rlen, glen, blen;
            int nc = sscanf(cname, "%x%n/%x%n/%x%n", &red, &rlen, &green, &glen, &blue, &blen);
            if (nc == 3) {
              blen -= glen + 1;
              glen -= rlen + 1;
              
              int rmax = ((unsigned) 1 << (unsigned)(4 * rlen)) -1;
              int gmax = ((unsigned) 1 << (unsigned)(4 * glen)) -1;
              int bmax = ((unsigned) 1 << (unsigned)(4 * blen)) -1;
              
              /* CCIR 601 luminance value */
              double luminance = 
                (0.299 * (double)red/(double)rmax) + 
                (0.587 * (double)green/(double)gmax) + 
                (0.114 * (double)blue/(double)bmax);
              
              printf("rgb:%s\n", cname);
              printf("luminance:%f\n", luminance);

              j += sizeof ST - 1;
              size_t saved_i = i;
              while (j < buflen) buf[i++] = buf[j++];
              buflen = i;
              i = saved_i;
              success = true;
            }
            free(cname);
          }
        }
      }

      // Check for response to the DSR
      if (i + sizeof CSI - 1 + 1 + sizeof DSR - 1  <= buflen &&
          memcmp(CSI, &buf[i], sizeof CSI - 1) == 0 &&
          buf[i + sizeof CSI - 1] >= '0' && buf[i + sizeof CSI - 1] <= '9' &&
          memcmp(DSR, &buf[i + sizeof CSI - 1 + 1], sizeof DSR - 1) == 0
         ) {
        size_t saved_i = i;
        for (size_t j = i + sizeof CSI - 1 + 1 + sizeof DSR - 1; j < buflen;) {
          buf[i++] = buf[j++];
        }
        buflen = i;
        i = saved_i;
        if (success) exit(EXIT_SUCCESS);
        err(errno=ENOSYS, "Terminal does not support OSC background color query");
      }
    }
  }
}
