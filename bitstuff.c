#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

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

#define YES 1
#define NO 0

void byte2bitstring(unsigned char byte, unsigned char *arr);
void showbitbuffer(const unsigned char *bitbuffer, int len,
                   unsigned int period, unsigned long int idx);
int bitstuffer(unsigned char *arr, unsigned int len,
                unsigned int period, int stuff, int verbose, unsigned char stf);
 
int main(int argc, char **argv){
  FILE *fd = stdin;
  unsigned int
    P = PER_DEFAULT,
    S = OPERATION_DEFAULT,
    verbose = VERBOSE_DEFAULT,
    readstdin = YES,
    binary = NO,
    raw = YES,
    inbytes = 0,
    dirty = NO,
    opt,
    afterlength = 0,
    wrap = NO;
  char filename[MAXFILENAMELENGTH] = "stdin\0";
  char format[8] = "%c";
  unsigned char stf = 0;
  
  if (argc < 2)
    goto help;
  
  while ((opt = getopt(argc, argv, "10usqbcd:vp:f:x")) != -1){
    switch (opt) {
    case 'u':
      S = NO;
      break;
    case 's':
      S = YES;
      break;
    case 'd':
      dirty = YES;
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
      stf = 1;
      break;
    case '0':
      stf = 0;
      break;
    case 'f':
      strncpy(filename,optarg,MAXFILENAMELENGTH);
      if (strcmp(filename,"-"))
        readstdin=0;
      break;
    case 'c':
      raw = YES;
      strncpy(format,"%c",2);
      break;
    case 'x':
      raw = NO;
      wrap = 80/3;
      strncpy(format,"%2.2x ",6);
      break;
    case 'b':
      raw = NO;
      wrap = 80/9;
      binary = YES;
      break;
    case 'v':
      verbose = YES;
      break;
    case 'q':
      verbose = NO;
      break;
    case 'h':
    default:
    help:
      fprintf(stderr, GREEN"BITSTUFFER UTILITY\n"
              YELLOW"=-=-=-=-==-=-=-=-=\n"COLOR_RESET
              "Usage: %s [OPTIONS] -p <period> -f <filename> \n"
              "-s: bitstuffing [default]\n"
              "-u: unstuffing\n"
              "-c: output bytes as ASCII characters [qdefault]\n"
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

  unsigned char *buffer = malloc(MAXBUFFERSIZE*2*sizeof(unsigned char));
  int bytecount = 0;
  char c;

  do {
    buffer[bytecount++] = fgetc(fd);  
  } while  (!feof(fd) && bytecount < MAXBUFFERSIZE && (!dirty || bytecount <= inbytes));

  buffer[bytecount-1] = '\0';
  //  int ham = bytecount;
  //bytecount += ham;

  /*** The main event ***/
  bytecount = bitstuffer(buffer, bytecount, P, S, verbose, stf);
  /**********************/

  int j=0;
  unsigned char ch, bitstring[8*sizeof(unsigned char)+1];
  while (j < bytecount && ((ch = buffer[j++]) != '\0') || dirty){
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
 * Returns the length of the new bytestring, and writes the new,
 * bit-stuffed array to the memory allocated to the initial array
 * (this is why we're careful to over- allocate that memory in
 * main()).
 ***/ 
int bitstuffer(unsigned char *arr, unsigned int len,
                unsigned int period, int stuff, int verbose, unsigned char stf){
  
  unsigned long int bitlen = len*32*sizeof(unsigned char);
  unsigned char *bitbuffer = malloc (bitlen * sizeof(char));
  memset(bitbuffer,0,bitlen);
  unsigned long int idx = 0;
  unsigned long int skips=0, p, i;
  unsigned char byte=0, bit=0;
  
  int tally = 0;
  int ok = YES;
  for (p = 0; p < len; p++){
    byte = *(arr + p);
    for (i=0; i < 8; i++){
      bit = byte & 1;
      //      fprintf(LOG, "BYTE: %2.2x     BIT: %d\n",byte,bit);
      if (ok)
        bitbuffer[idx++] = bit;
      else
        ok = YES;  // it's okay, now because we discarded a zero
      tally += bit;
      tally *= bit;
      ok = (tally != period) || stuff || !period;
      if (period && tally == period){
        tally = 0;
        // insert an extra 0 if stuffing, but skip next bit if unstuffing
        if (stuff)
          bitbuffer[idx++] = stf;
         
      }
      byte >>= 1;

      
      
    }
    if (verbose) showbitbuffer(bitbuffer,idx, period*stuff, 0);
  }
    
  
  
  if (verbose) fprintf(LOG,"---\n");
  
  p = 0;
  bitlen = idx;
  idx = 0;
  
  while (idx < bitlen){
    if (verbose)
      showbitbuffer(bitbuffer, bitlen, period*stuff, idx);
    byte = '\0';
    for (i = 0; i < 8; i++){
      byte <<= 1;
      byte += bitbuffer[idx + (7-i)];
    }
    idx += 8;         // hop to the next byte
    *(arr + p++) = byte;
  }
  
  *(arr + p) = '\0';  // terminate string with null byte

  free(bitbuffer);
  return p;
}

/***
 * This function is primarily for debugging purposes. It can be activated by passing the verbose
 * flag to the programme from the command line (-v).
 * When activate, this will display the contents of the "bitbuffer" (the buffer filled with bytes
 * masquerading as bits, for ease of manipulation), as a series of *little-endian*, byte-sized
 * binary numbers. It also highlights the stuffed bits in magenta, which is nice. 
 * 
 * @param const unsigned char *bitbuffer: the array of 1s and 0s (not '1's and '0's) to display
 *        int len: the length of that array
 *        unsigned int period: same as in bitstuffer(): the frequency of the stuffed bits 
 *        unsigned long int idx: the current index to the active cell of the bitbuffer
 ***/
void showbitbuffer(const unsigned char *bitbuffer, int len, unsigned int period, unsigned long int idx){
  int  i, tally = 0;
  char bit;
  int highlight = NO;
  
  len -= idx;
  for (i = 0; i < len; i++){
    bit = *(bitbuffer + idx + i);
    tally += bit;
    tally *= bit;


    if (i > 0 && i % 8 == 0)
      fprintf(LOG," ");
    fprintf(LOG,"%s%d%s", highlight? MAGENTA:"",bit,COLOR_RESET);
    highlight = NO;
    if (period && period == tally){
      highlight = YES;
      tally = 0;
    }
      }
  fprintf(LOG,"\n");
  return;
}

/***
 * Converts a byte to a readable string of '1' and '0' chars.
 *
 * @param unsigned char byte: the byte to convert
 *        unsigned char *arr: the array to which to write
 ***/
void byte2bitstring(unsigned char byte, unsigned char *arr){
  int i;
  for (i=7; i >= 0; i--){ // other direction? probably. 
    *(arr + i) = '0'+(byte & 1);
    byte >>= 1;
  }
}

void properstuffer(unsigned char *arr, unsigned int len,
                   unsigned int period, int stuff, int verbose){
  

}
