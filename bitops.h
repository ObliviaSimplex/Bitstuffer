#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define ON 1
#define OFF 0


void setbit(unsigned char *byte, unsigned long int index,
            unsigned char bit){
  bit %= 2;
  unsigned int byte_index = index / 8;
  unsigned char bit_index = index % 8;
  char mask = 1 << bit_index;
  *(byte + byte_index) = bit?
    *(byte + byte_index) | mask :
    *(byte + byte_index) & ~mask;
}

void flipbit(unsigned char *byte, unsigned char index){
  unsigned int byte_index = index / 8;
  unsigned char bit_index = index % 8;
  char mask = 1 << bit_index;
  *(byte + byte_index) ^= mask;
}

unsigned char getbitasormask(const unsigned char *byte,
                             unsigned char index){
  unsigned int byte_index = index / 8;
  unsigned char bit_index = index % 8;
  unsigned char mask = 1 << bit_index;
  return (*(byte + byte_index) & mask) ;
}

unsigned char getbit(const unsigned char *byte, unsigned char index){
  return !!(getbitasormask(byte, index));
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







