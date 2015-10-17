#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

void setbit(unsigned char *byte, unsigned char index,
            unsigned char bit){
  bit %= 2;
  char mask = 1 << index;
  *byte = bit? *byte | mask : *byte & ~mask;
}

void flipbit(unsigned char *byte, unsigned char index){
  char mask = 1 << index;
  *byte ^= mask;
}

unsigned char getbitasormask(unsigned char *byte,
                             unsigned char index){
  unsigned char mask = 1 << index;
  return (*byte & mask) ;
}

unsigned char getbit(unsigned char *byte, unsigned char index){
  return (getbitasormask(byte, index)) >> index;
}

/***
 * Converts a byte to a readable string of '1' and '0' chars.
 *
 * @param unsigned char byte: the byte to convert
 *        unsigned char *arr: the array to which to write
 ***/
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
  int i=0, e=0;
  unsigned char byte=0;
  while (i < 8){
    ( byte += ( *(bitarray + 7-i) << i++) );
  }
  return byte;
}

bitarray2bytearray(char *bitarray, char *bytearray, int len){
  int bitindex = 0, byteindex = 0;

  while (byteindex * 8 < len)
    *(bytearray + byteindex) = bitarray2byte((bitarray + (8 * byteindex++)));
  
}







