#include "bitops.h"

int main(int argc, char **argv){
  unsigned char byte;
  printf("Enter byte > ");
  scanf("%c",&byte);
  char bitstring[9];
  
  byte2bitstring(byte, bitstring);
  printf("BYTE: %s     0x%2.2x\n",bitstring, byte);
  unsigned short int index;
  printf("Peek index > ");
  scanf("%hd",&index);

  unsigned short int bit;
  bit = getbit(&byte, index);
  printf("Bit at index %hd: %hd\n", index, bit);

  printf("Flip index > ");
  scanf("%hd",&index);
  flipbit(&byte,index);

  byte2bitstring(byte, bitstring);
  printf("BYTE: %s     0x%2.2x\n",bitstring, byte);

  printf("Set %%hd to %%hd > ");
  scanf("%hd%hd",&index, &bit);

  setbit(&byte,index, bit);
  byte2bitstring(byte,bitstring);
  printf("BYTE: %s     0x%2.2x\n",bitstring, byte);

  printf("Testing conversion functions...\n");

  unsigned char byte2;
  bitstring2bitarray(bitstring,8);
  byte2 = bitarray2byte(bitstring);

  
  printf("%2.2x == %2.2x ?  %s!\n",byte, byte2, ((byte == byte2)? "SUCCESS" : "FAILURE"));
  
  return 0;
}




