#include "bitops.h"
#include "unistd.h"


#define DEFAULT_GEN 0x0b // in decimal: 11; in binary: 1011
#define BINARY  0
#define DECIMAL  1
#define HEX  2
#define ASCII  3
#define MINARGS 0
#define MAXLEN 0x10000
#define DEFAULT_GENERATOR 0x04C11DB7


#define SEND 1
#define RECV 0

bitarray_t * CRC(bitarray_t *message,
                 uint32_t generator,
                 unsigned char send);

int verbose = 1;

int main(int argc, char **argv){

  FILE *fd = stdin;
  char fmt[8];
  int inputformat = 0;
  uint32_t input;
  char opt;
  char direction = SEND;
  char input_as_binary = FALSE;
  int burst_length = 0;
  char random_msg = TRUE;
  int random_msg_size = 1520;
  uint32_t generator = DEFAULT_GENERATOR;
  
  if (argc < MINARGS)
    goto help;
  while ((opt = getopt(argc, argv, "bvqg:ce:srh")) != -1){
    switch(opt) {
    case 'b':
      input_as_binary = TRUE;
      break;
    case 'c':
      input_as_binary = FALSE;
      break;
    case 'g':
      generator = atoi(optarg);
      break;
    case 'v':
      verbose = TRUE;
      break;
    case 'q':
      verbose = FALSE;
      break;
    case 'R':
      random_msg = TRUE;
      break;
    case 'e':
      burst_length = atoi(optarg);
      break;
    case 's':
      direction = SEND;
      break;
    case 'r':
      direction = RECV;
      break;

    case 'h':
    default:
    help:
      printf("Help message...\n");
      break;
    }
  }
  
  if (is_big_endian()){
    fprintf(stderr, "Warning: this is a big-endian system.\n"
            "This programme may not work as expected.\n");
  }
  
  bitarray_t *bitmsg;
  
  if (input_as_binary == FALSE){ 
    char *txtmsg = read_characters(fd, EOF);
    bitmsg = make_bitarray(txtmsg, strlen(txtmsg));
    free(txtmsg);
  } else {
    bitmsg = read_binary(fd);
  }
  

  //char inputstring[0x1000];
  //scanf("%s",inputstring);
  //bitmsg = make_bitarray(inputstring, strlen(inputstring));
  
  fprintf(stderr,"MESSAGE READ: %s\n",bitmsg->array);
  fprintf(stderr,"IN BINARY:    ");
  print_bitarray(stderr, bitmsg);
  fprintf(stderr,"\n");

  bitarray_t *newmsg = CRC(bitmsg, generator, SEND);
  bitarray_t *recvmsg = CRC(newmsg, generator, RECV);
  
  destroy_bitarray(newmsg);
  destroy_bitarray(bitmsg);
  destroy_bitarray(recvmsg);
  
  return 0;
}


bitarray_t * CRC(bitarray_t *message,
                    uint32_t generator,
                    unsigned char send){

  int shiftbitlen = 0;
  uint32_t g = generator;
  while ((g >>= 1) != 0)
    shiftbitlen ++;

  // It will be helpful to have a mask for grabbing the high bit of the reg.
  uint32_t shiftreg_highmask = (0x1 << (shiftbitlen-1));
  uint32_t shiftreg_cropmask = ~(0xffff << (shiftbitlen));
  // We drop the MSB of the generator when determining our xor gates.
  uint32_t xorplate = generator & shiftreg_cropmask;
  
  /////////
  if (verbose){
    fprintf(stderr, "xorplate: ");
    fprint_lint_bits(stderr, xorplate);
    fprintf(stderr,"\n");
  }
  //////////
  

  bitarray_t *bitmsg_out;
  fprintf(stderr,"message->end = %d\n",message->end);
  bitmsg_out = calloc(1,sizeof(bitarray_t));
  bitmsg_out->array = calloc((message->end/8) * 2, sizeof(char));
  bitmsg_out->end = message->end;
  bitmsg_out->residue = 0;
  
  uint32_t bit_index = 0;

  chunky_integer_t shiftreg;
  memset(&shiftreg,0,sizeof(uint32_t));
  
  char *shiftreg_string;
  char *msg_bitstring;
  char xored = 0;
  uint32_t topbit = 0, bit = 0;
  
  fprintf(stderr,"IN:  ");
  print_bitarray(stderr, message);
  fprintf(stderr,"\n");

  int iters = (send == SEND)?
    bitmsg_out->end + shiftbitlen :
    bitmsg_out->end;

   //???
  while (bit_index < iters){

    bit = getbit(message->array, bit_index++);
    shiftreg.integer = (shiftreg.integer << 1) & shiftreg_cropmask;
    shiftreg.integer |= bit;
    // When the MSB of the shift register is 1, perform XOR operation
    if (topbit){
      shiftreg.integer ^= xorplate; ///xorplate;
      if (verbose) xored = 1;
    } 
  
    topbit = shiftreg.integer & shiftreg_highmask;

    if (verbose){ 
      shiftreg_string = stringify_chunky(&shiftreg, shiftbitlen);
      fprintf(stderr, "[%2.2d] SHIFTREG: %s  FED: %d  %s\n", bit_index,
              shiftreg_string, bit,
              xored? "XOR EVENT" : "");
      free(shiftreg_string);
      xored = 0;
    }
  }

  if (is_big_endian())
    shiftreg.integer = end_reverse(shiftreg.integer);
  // copy shiftregister contents onto end of message;
  memcpy(bitmsg_out->array, message->array,
         message->end/8 + 1);
  //bitmsg_out->array = message->array;
  
  bitmsg_out->residue = shiftreg.integer;

  fprintf(stderr,"bitmsg_out->end = %d\n",bitmsg_out->end);
  int i;
  if (send == SEND) {
    for (i = shiftbitlen-1; i >= 0; i --){
      //for (i=0; i < shiftbitlen; i++){
      setbit(bitmsg_out->array, bitmsg_out->end++,
             getbit(shiftreg.bytes,i));
      
      fprintf(stderr, "(%d) copying %d from shiftreg to bitmsg_out bit #%d\n",
              i,getbit(shiftreg.bytes,i), bitmsg_out->end-1);
     }
  }

  if (send == RECV) {
    if (!bitmsg_out->residue)
      fprintf(stderr, "NO CORRUPTION DETECTED.\n");
    else{
      fprintf(stderr, "*** CORRUPTION DETECTED ***\n");
      fprintf(stderr, "*** RESIDUE: 0x%lx\n",bitmsg_out->residue);
    }
  }

  /*
  fprintf(stderr, "Value in shiftreg: %x\n", shiftreg.integer);
  fprintf(stderr, "First byte in shiftreg: %x\n", shiftreg.bytes[0]);
  fprintf(stderr, "message->end: %d (%x); bitmsg_out->end: %d (%x)\n",
          message->end, getbit(message->array, message->end),
          bitmsg_out->end, getbit(bitmsg_out->array, bitmsg_out->end));
  fprintf(stderr, "About to send back: %s\n",bitmsg_out->array);
  */
  if (verbose){
    fprintf(stderr,"IN:  ");
    print_bitarray(stderr, message);
    fprintf(stderr,"\n");
    fprintf(stderr,"OUT: ");
    print_bitarray(stderr, bitmsg_out);
    fprintf(stderr,"\n\n");
  }
  return bitmsg_out;
}





























