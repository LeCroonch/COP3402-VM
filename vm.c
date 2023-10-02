#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "bof.h"
#include "instruction.h"
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
 
     if(strcmp(argv[1], "-p") == 0){

          char* fileName = argv[2];
 
          BOFFILE bf = bof_read_open(fileName);
          
          BOFHeader bh = bof_read_header(bf);
          printf("%d\n", bh.data_start_address);
          printf("%-5s: %s\n", "Addr", "Instruction");
          for(int i = 0; i < bh.text_length/BYTES_PER_WORD; i++){
      
               bin_instr_t bi = instruction_read(bf);     
               memory.instrs[i] = bi;
               printf("   %d %s\n", PC, instruction_assembly_form(memory.instrs[i]));
               PC = PC + 4;
          }
          
          if((bh.data_length/BYTES_PER_WORD) == 0){
               printf("%-5u: %d\n", bh.data_start_address, memory.words[0]);   
          }
          
          

          for(int i = 0; i < bh.data_length/BYTES_PER_WORD; i++){

               printf("%-5u: %d\n", bh.data_start_address, memory.words[i]);
               bh.data_start_address = bh.data_start_address + 4;
          }

          

         
     } else {
 
          BOFFILE bf = bof_read_open(argv[1]);

          BOFHeader bh = bof_read_header(bf);

       
          for(int i = 0; i < bh.data_length/BYTES_PER_WORD; i++){
     
               memory.words[bh.data_start_address + i] = bof_read_word(bf);

          }
               
          for(int i = 0; i < bh.data_length/BYTES_PER_WORD; i++){

               printf("a%d;\t%s", PC, instruction_assembly_form(memory.instrs[i]));
               PC = PC + 4;
          }
     }

    
}

void instruction(int PC){


     PC = PC + 4;
}