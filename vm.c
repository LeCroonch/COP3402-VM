#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "regname.h"
#include "machine_types.h"
#include "instruction.h"
// a size for the memory (2^16 bytes = 64K)
#define MEMORY_SIZE_IN_BYTES (65536 - BYTES_PER_WORD)
#define MEMORY_SIZE_IN_WORDS (MEMORY_SIZE_IN_BYTES / BYTES_PER_WORD)

static union mem_u
{
     byte_type bytes[MEMORY_SIZE_IN_BYTES];
     word_type words[MEMORY_SIZE_IN_WORDS];
     bin_instr_t instrs[MEMORY_SIZE_IN_WORDS];
} memory;


int main(int argc, char * argv[]){

     int PC;
     word_type reg[NUM_REGISTERS];

     if(strcmp(argv[1], "-p")){

          BOFFILE bf = bof_read_open(argv[2]);
          
          BOFHeader bh = bof_read_header(bf);
          
          for(PC = 0; PC < bh.text_length/BYTES_PER_WORD; PC++){

               bin_instr_t bi = instruction_read(bf);     
               memory.instrs[PC] = bi;

          }

          for(int i = 0; i < bh.data_length/BYTES_PER_WORD;){

               memory.words[bh.data_start_address + i] = bof_read_word(bf);

          }
          
          for(int i = 0; i < bh.data_length/BYTES_PER_WORD; i++){
               instruction_print( , , memory.instrs[i]);
          }


     } else {

      

     }

    
}