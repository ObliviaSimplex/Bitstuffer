#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

#define ON 1
#define OFF 0

#define TRUE 1
#define FALSE 0

#define LEFT  +1
#define RIGHT -1

#define BIG 0x0b
#define LITTLE 0x00


typedef struct bitarray {

  uint8_t *array;
  uint32_t end; // bit index of last bit + 1
  uint32_t residue;
} bitarray_t;
  

typedef union chunky_integer {
  uint32_t integer;
  unsigned char bytes[4];
} chunky_integer_t;

int is_big_endian(void){
  chunky_integer_t checker;
  checker.integer = 0x0bffff00;
  return (checker.bytes[0] == BIG);
}

bitarray_t * make_bitarray(char *arr, int len){
  bitarray_t *ba = calloc(1,sizeof(bitarray_t));
  ba->array = calloc(len, sizeof(char));
  memcpy(ba->array, arr, len);
  ba->end = len*8;
  ba->residue = 0;
  return ba;
}

void destroy_bitarray(bitarray_t *ba){
  free(ba->array);
  free(ba);
}

void setbit(unsigned char *byte, uint32_t index,
            unsigned char bit){
  bit %= 2;
  unsigned long int byte_index = index / 8;
  unsigned char bit_index = index % 8;
  char mask = 1 << bit_index;
  *(byte + byte_index) = bit?
    *(byte + byte_index) | mask :
    *(byte + byte_index) & ~mask;
}

void flipbit(unsigned char *byte, unsigned long int index){
  unsigned int byte_index = index / 8;
  unsigned char bit_index = index % 8;
  char mask = 1 << bit_index;
  *(byte + byte_index) ^= mask;
}

unsigned char getbitasormask(const unsigned char *byte,
                             unsigned long int index){
  unsigned long int byte_index = index / 8;
  unsigned long int bit_index = index % 8;
  unsigned char mask = 1 << bit_index;
  return (*(byte + byte_index) & mask) ;
}

unsigned char getbit(const unsigned char *byte, unsigned long int index){
  return !!(getbitasormask(byte, index));
}

void bitarray_push(bitarray_t *ba, unsigned char bit){
  setbit(ba->array, (ba->end)++, bit);
}

unsigned char bitarray_pop(bitarray_t *ba){
  if (!ba->end){
    fprintf(stderr,"ERROR: Attempt to pop an empty bitarray.\n");
    exit(EXIT_FAILURE);
  }
    
  return getbit(ba->array, --(ba->end));
}

/***
 * Converts a byte to a readable string of '1' and '0' chars.
 *
 * @param unsigned char byte: the byte to convert
 *        int len: length of the byte array to convert, in bytes
 * @return pointer to bitstring, on the heap. Free after using. 
 ***/
char * bytes2bitstring(const unsigned char *byte, int len){
  int i;
  unsigned char *bytearray;
  bytearray = malloc (sizeof(char)*len);
  memcpy(bytearray,byte, len);
  int byte_index = 0;
  int spaces = 0;
  char *arr = malloc((len*9 + 1) * sizeof(bytearray));
  while (byte_index < len){
    for (i=7; i >= 0; i--){
      *(arr + spaces + byte_index*8 + i) = '0'+(*(bytearray + byte_index) & 1);
      *(bytearray + byte_index) >>= 1;
    }
    if (byte_index < len-1)
      *(arr + spaces++ + byte_index*8 + 8) = ' ';
    byte_index ++;
  }
  *(arr + spaces + byte_index*8 + 8) = '\0';
  free(bytearray);
  return arr;
}

void byte2bitstring(unsigned char byte, unsigned char *arr){
  int i;
  
  for (i=7; i >= 0; i--){
    *(arr + i) = '0'+(byte & 1);
    byte >>= 1;
  }
}

void byte2bitarray(unsigned char byte, unsigned char *arr){
  int i;
  for (i = 7; i >= 0; i--){
    *(arr + i) = (byte & 1);
    byte >>= 1;
  }
}

void bitstring2bitarray(char *bitstring, int len){
  // transforms in place
  int i;
  for (i = 0; i < len; *(bitstring + i++) -= '0');
}

void bitarray2bitstring(char *bitarray, int len){
  int i;
  for (i = 0; i < len; *(bitarray + i++) += '0');
}

unsigned char bitarray2byte(char *bitarray){
  // here we assume that bitstring is exactly eight characters long
  // and composed of the literal characters '0' and '1'
  // and has the most significant bit on the left
  int i=0;
  unsigned char byte=0;
  int size = 8;
  
  while (i < size){
    ( byte += ( *(bitarray + 7-i) << i) );
    i++;
  }
  return byte;
}

void bitarray2bytearray(char *bitarray, char *bytearray, int len){
  int byteindex = 0;

  while (byteindex * 8 < len){
    *(bytearray + byteindex) =
      bitarray2byte((bitarray + (8 * byteindex)));
    byteindex++;
  }
  return;
}



void burst_error(unsigned char *message, int msglen,
                 int errbitlen, int highlow){

  int errbytes = errbitlen / 8;
  unsigned char errbytemask = 0xffff << (errbitlen % 8);

  int index = rand() % (msglen-errbytes-1);

  memset(message+index, highlow, errbytes);

  *(message + index + errbytes) &= errbytemask;

  return;
}


char * longint2bytearray (uint32_t l) {
  char * bytearray = calloc((int) sizeof(uint32_t), sizeof(char));
  
  union chunky_integer *chunky;
  chunky = malloc(sizeof(uint32_t));
  chunky->integer = l;

  return chunky->bytes;
}

char * longint2bitstring (uint32_t l){
  return bytes2bitstring(longint2bytearray(l), (int) sizeof(uint32_t));
}

uint32_t end_reverse (uint32_t l){
  uint32_t r = 0;
  uint32_t mask = 0xff;
  uint8_t i = 0;
  for (i = 0; i < 4; i++){
    r <<= 8;
    r |= (mask & l) >> (i*8);
    mask <<= 8;    
  }
  return r;
}


void print_bitarray_bytes (FILE *channel, bitarray_t *ba){
  char * s = bytes2bitstring(ba->array, ba->end/8 + 1);//
  //  ((ba->end % 8)? 0 : 1));
  fprintf(channel, "%s", s);
  free(s);
}

void print_lintbits (FILE *channel, uint32_t l){
  char * s = longint2bitstring(l);
  fprintf(channel, "%s",s);
  free(s);
}

void print_bitarray (FILE *channel, bitarray_t *ba){
  int i = 0;
  while (i < ba->end)
    fprintf(channel, "%d", getbit(ba->array, i++));
}

char * stringify_bitarray (const bitarray_t *ba){
  char * s;
  s = calloc (1, sizeof(ba->array));
  int i = 0;
  while (i < ba->end)
    *(s + i) = getbit(ba->array, i++) + '0';
  return s;
}

char * stringify_chunky (const chunky_integer_t *ci, int bitlen){
  char * s;
  s = calloc (bitlen, sizeof(char));
  int i = 0;
  chunky_integer_t *standin = calloc (1, sizeof(chunky_integer_t));
  standin->integer = is_big_endian()? end_reverse(ci->integer) : ci->integer; 
  while (i < bitlen)
    *(s + i) = getbit(standin->bytes, i++) + '0';
  free(standin);
  return s;                     
}


/**
 * Reads a series of ASCII '0's and '1's as a binary stream.
 **/
bitarray_t * read_binary (FILE *channel){
  int size = 0x100;
  char endsig = '\n';
  uint8_t * array = calloc(size,sizeof(uint8_t));
  int b = 0, i = 0;
  char glyph;
  while (((glyph = fgetc(channel)) != endsig) && (!feof(channel))){
    i = b/8; b++;
    array[i] <<= 1;
    array[i] |= (glyph-'0');
    printf("reading %c -- ",glyph);
    printf("array[%d] = %2.2x\n",i,array[i]);
    if (i >= (size*3)/4){
      size *= 2;
      uint8_t *copy = calloc(size,sizeof(uint8_t));
      memcpy(copy, array, i);
      free(array);
      array = copy;
    }
  };
  i++;
  bitarray_t *ba = calloc(1, sizeof(bitarray_t));
  ba->array = calloc(i,sizeof(char));
  memcpy(ba->array, array,i);
  ba->end = b;
  ba->residue = 0;
  return ba;  
}

char * read_characters (FILE *channel, char endsig){
  long int size = 0x100;
  char * string = calloc(size,sizeof(uint8_t));
  int i = 0;
  char glyph;
  do {
    //printf("*********************\n");
    glyph = fgetc(channel);
    string[i++] = glyph;
    if (i >= (size*3)/4){
      size *= 2;
      uint8_t *copy = calloc(size,sizeof(uint8_t));
      memcpy(copy, string, i);
      free(string);
      string = copy;
    }
  } while (!feof(channel) && glyph != endsig);

  string[i] = '\0';
  if (glyph == endsig) string[i-1] = '\0';
  return string;
}

char * read_n_characters (FILE *channel, int n){
  long int size = 0x100;
  char * string = calloc(size,sizeof(uint8_t));
  int i = 0;
  char glyph;
  do {
    //printf("*********************\n");
    glyph = fgetc(channel);
    string[i++] = glyph;
    if (i >= (size*3)/4){
      size *= 2;
      uint8_t *copy = calloc(size,sizeof(uint8_t));
      memcpy(copy, string, i);
      free(string);
      string = copy;
    }
  } while (!feof(channel) && i < n);

  string[i] = '\0';
  // if (glyph == endsig) string[i-1] = '\0';
  return string;
}

char * get_random_bytes(int len){
  FILE *urandom = fopen("/dev/urandom","r");
  return read_n_characters(urandom, len);
}


void fprint_lint_bits(FILE *channel, long int l){
  while (l != 0){
    int bit = l & 1;
    l >>= 1;
    fprintf(channel, "%d",bit);
  }
}


