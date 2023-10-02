#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "utilities.h"
#include "bof.h"
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

const char *instructionCycle(bin_instr_t instr);
void storeInstrs(memory* mem, char* fileName);

int main(int argc, char * argv[]){

     int PC = 0;
     
     word_type reg[NUM_REGISTERS];
 
     if(strcmp(argv[1], "-p") == 0){

          char* fileName = argv[2];
 
          BOFFILE bf = bof_read_open(fileName);
          
          BOFHeader bh = bof_read_header(bf);
          memory mem1;
          printf("%-5s: %s\n", "Addr", "Instruction");
          for(int i = 0; i < bh.text_length/BYTES_PER_WORD; i++){
      
               bin_instr_t bi = instruction_read(bf);     
               mem1.instrs[i] = bi;
               
               printf("%-5u: %s\n", PC, instruction_assembly_form(memory.instrs[i]));
               PC = PC + 4;
          }

          
          int currAddr = bh.data_start_address;
          
          for(int i = bh.data_start_address; i < bh.stack_bottom_addr; i+4){

               printf("%-5u: %d", currAddr, memory.words[currAddr/BYTES_PER_WORD]);

               if(memory.bytes[i] == 0){
                    printf(" ...\n");
                    break;

               }
               
               currAddr = currAddr + 4;               
          }

     } else {
 
          BOFFILE bf = bof_read_open(argv[1]);

          BOFHeader bh = bof_read_header(bf);

       
          for(int i = 0; i < bh.data_length/BYTES_PER_WORD; i++){
     
               bin_instr_t bi = instruction_read(bf);
               memory.instrs[i] = bi;
               instructionCycle(memory.instrs[i]);
          }
               
     }

    
}

const char *instructionCycle(bin_instr_t instr){
     char *buf;
     char * name;
     instr_type it = instruction_type(instr);
     printf("%d\n", it);
     switch(it){
          case syscall_instr_type:
          break;
          case reg_instr_type:
          switch (instr.reg.func){
               
              case ADD_F:
                  register[instr.reg.rd] = register[instr.reg.rs] + register[instr.reg.rt];
               break;
               case SUB_F:
                   register[instr.reg.rd] = register[instr.reg.rs] - register[instr.reg.rt];
               break;
               case MUL_F:
                   // HI LO multiplication implementation

               break;
               case DIV_F:
                   //same but for division
               break;
               case MFHI_F:

               break;
               case MFLO_F:
               break;
               case AND_F:
               break;
               case BOR_F:
               break;
               case NOR_F:
               break;
               case XOR_F:
               break;
               case SLL_F:
               break;
               case SRL_F:
               break;
               case JR_F:
               break;
               case SYSCALL_F:
               //table 6 System Calls
               break;

          }
          case immed_instr_type:
          break;
          case jump_instr_type:
          break;
          default:
          bail_with_error("Uknown instruction type (%d) in instruction_assembly_form!", it);

          return buf;
     }

   
}