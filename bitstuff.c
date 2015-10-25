#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "bitops.h"

#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"
#define CYAN    "\x1b[36m"
#define MAGENTA "\x1b[35m"
#define COLOR_RESET   "\x1b[0m"

#define PER_DEFAULT 5
#define MAXFILENAMELENGTH 64
#define OPERATION_DEFAULT 1 // stuff
#define VERBOSE_DEFAULT 0
#define LOG stderr
#define MAXBUFFERSIZE 0x10000  // 64K


void byte2bitstring(unsigned char byte, unsigned char *arr);
void showbitbuffer(const unsigned char *bitbuffer, int len,
                   unsigned int period, uint32_t idx);
unsigned char * bitstuffer(const unsigned char *arr, unsigned int len,
                           unsigned int period, int stuff, int verbose,
                           unsigned char stuffingbit);
 
int main(int argc, char **argv){
  FILE *fd = stdin;
  unsigned int
    P = PER_DEFAULT,
    S = OPERATION_DEFAULT,
    verbose = VERBOSE_DEFAULT,
    readstdin = ON,
    binary = OFF,
    raw = ON,
    inbytes = 0,
    dirty = OFF,
    opt,
    wrap = OFF;
  char filename[MAXFILENAMELENGTH] = "stdin\0";
  char format[8] = "%c";
  unsigned char stuffingbit = 0;
  
  if (argc < 2)
    goto help;
  
  while ((opt = getopt(argc, argv, "10usqbcd:p:f:x")) != -1){
    switch (opt) {
    case 'u':
      S = OFF;
      break;
    case 's':
      S = ON;
      break;
    case 'd':
      dirty = ON;
      inbytes = atoi(optarg);
      break;
    case 'p':
      P = atoi(optarg);
      if (P == 1){
        fprintf(stderr,"ERROR: Period must be 2 or greater. Otherwise, you're just stuffing\n"
                "every bit. The system will grow angry, and heaps will overflow.\n"
                "The exception to this rule is that a period of 0 may be used to\n"
                "turn off stuffing.\n");
        exit(EXIT_FAILURE);
      }
      break;
    case '1':
      stuffingbit = 1;
      break;
    case '0':
      stuffingbit = 0;
      break;
    case 'f':
      strncpy(filename,optarg,MAXFILENAMELENGTH);
      if (strcmp(filename,"-"))
        readstdin=0;
      break;
    case 'c':
      raw = ON;
      strncpy(format,"%c",2);
      break;
    case 'x':
      raw = OFF;
      wrap = 80/3;
      strncpy(format,"%2.2x ",6);
      break;
    case 'b':
      raw = OFF;
      wrap = 80/9;
      binary = ON;
      break;
    case 'q':
      verbose = OFF;
      break;
    case 'h':
    default:
    help:
      fprintf(stderr, GREEN"BITSTUFFER UTILITY\n"
              YELLOW"=-=-=-=-==-=-=-=-=\n"COLOR_RESET
              "Usage: %s [OPTIONS] -p <period> -f <filename> \n"
              "-s: bitstuffing [default]\n"
              "-u: unstuffing\n"
              "-c: output bytes as characters [default]\n"
              "-x: output bytes in hexidecimal format\n"
              "-b: output bytes in binary format\n"
              "-f <filename> : name of file to bitstuff.\n"
              "-f - : read from stdin [default]\n"
              "-h: display this help menu.\n",
              argv[0]);
      exit(EXIT_FAILURE);
    }  
  }
  
  if (!readstdin && (fd = fopen(filename,"r")) == NULL){
    fprintf(stderr,"Fatal error opening %s.\n", filename);
    exit(EXIT_FAILURE);
  }
  /*
  unsigned char *buffer = malloc(MAXBUFFERSIZE*2*sizeof(unsigned char));
  int bytecount = 0;

  do {
    buffer[bytecount++] = fgetc(fd);  
  } while  (!feof(fd) && bytecount < MAXBUFFERSIZE && (!dirty || bytecount <= inbytes));
  */
  unsigned char *buffer = read_characters(fd, EOF);
  int bytecount = strlen(buffer);
  
  //  buffer[bytecount-1] = '\0';

  /*** The main event ***/
  unsigned char *stuffed;
  stuffed = bitstuffer(buffer, bytecount, P, S, verbose, stuffingbit);
  bytecount = strlen(stuffed);
  /**********************/

  int j=0;
  unsigned char ch, bitstring[8*sizeof(unsigned char)+1];
  while ((j < bytecount && ((ch = stuffed[j++]) != '\0')) || dirty){
    if (binary) {
      memset(bitstring,0,9);
      byte2bitstring(ch, bitstring);
      printf("%s ",bitstring);
    } else {
      printf(format,ch);
    }
    if (wrap && j % wrap == 0)
      printf("\n");
  }
  if (!raw) printf("\n");

  free(stuffed);
  free(buffer);
  return 0;
}

/***
 * This is the workhorse of the utility. It handles both stuffing and 
 * unstuffing operations (the two being essentially similar, and easy to
 * differentiate with a single parameter (the boolean variable, stuff)).
 *  
 * @args unsigned char *arr: the string, or bytearray, to stuff
 *       unsigned int len: length of the string
 *       unsigned int period: the frequency at which to stuff bits
 *       int stuff: a boolean flag: 1 to stuff, 0 to unstuff
 *       int verbose: another boolean flag: 1 for verbose, 0 for quiet
 *
 * @returns a heap-allocated pointer to a new unsigned character array,
 *          storing a bitstuffed transformation of the initial array. 
 ***/ 
unsigned char * bitstuffer(const unsigned char *arr, unsigned int len,
                           unsigned int period, int stuff, int verbose,
                           unsigned char stuffingbit) {
  
  uint32_t bitlen = period ?
    len + len/period + len%period :
    len;
  uint32_t bitindex = 0;
  uint32_t p, i;
  unsigned char byte=0, bit=0;
  int tally = 0;
  unsigned char ok = 1;
  
  unsigned char *bitbuffer;
  if ((bitbuffer = calloc (bitlen, sizeof(char))) == NULL){
    fprintf(stderr, "Fatal error allocating memory for bitbuffer.\n"
            "Exiting.\n");
    exit(EXIT_FAILURE);
  }

  for (p = 0; p < len; p++){
    byte = *(arr + p);
    for (i=0; i < 8; i++){
      bit = byte & 1; // get the least significant bit off the byte
      byte >>= 1;     // and then shift the byte over. 
      if (ok)         // unless skipping the bit, go ahead and copy bit
        setbit(bitbuffer, bitindex++, bit);
      else 
        ok = 1;  // it's okay now, because we discarded a zero
      tally += stuffingbit? !bit : bit; // increment tally if bit is 1 (or 0)
      tally *= stuffingbit? !bit : bit; // reset tally if bit is 0     (or 1)
      ok = (tally != period) || stuff || !period;
      if (period && tally == period){
        tally = 0;
        if (stuff) // if stuffing (and not unstuffing), add stuffingbit now.
          setbit(bitbuffer, bitindex++, stuffingbit); 
      } // do nothing if unstuffing. Just skip this bit. 
    }
  }
  
  return(bitbuffer);
}



