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

const char *instructionCycle(bin_instr_t instr, int *PC, int *HI, int *LO, word_type* GPR);
void storeInstrs(BOFHeader* bh0,char* fileName);
void storeData(char* fileName);
void printData(BOFHeader* bh);

int main(int argc, char * argv[]){

     int PC = 0;
     BOFHeader bh;
     word_type GPR[NUM_REGISTERS];
     int HI = 0;
     int LO = 0;
 
     if(strcmp(argv[1], "-p") == 0){

         storeInstrs(&bh ,argv[2]);

         printf("%4s %s\n", "Addr", "Instruction");
         for(int i = 0; i < bh.text_length/BYTES_PER_WORD; i++){
               printf("%4d %s\n", PC, instruction_assembly_form(memory.instrs[i]));
               PC = PC + 4;
         }
         printData(&bh);


     } else {

         storeInstrs(&bh, argv[1]);

         for(int i = 0; i < bh.data_length/BYTES_PER_WORD; i++){
     
               bin_instr_t bi = instruction_read(bf);
               memory.instrs[i] = bi;
               instructionCycle(memory.instrs[i], &PC, &HI, &LO, &GPR);
         }
               
     }


}

void printData(BOFHeader* bh){
    bool noDots = true;
    int currAdd;
    for(int i = bh->data_start_address; i < bh->stack_bottom_addr; i++){
        if((i - bh->data_start_address) != 0 && (i - bh->data_start_address) % 5 == 0){
            printf("\n");
        }
        if(memory.words[i] != 0){
            currAdd = bh->data_start_address + (4 * (i-bh->data_start_address));
            printf("%8d: %-4d", currAdd, memory.words[i]);
        }else if(memory.words[i] == 0){
            currAdd = bh->data_start_address + (4 * (i-bh->data_start_address));
            if(noDots){
                printf("%8d: %-4d", currAdd, 0);
                printf("\t...\n");
                noDots = false;
            }else{
                break;
            }
        }
    }
}

void storeInstrs(BOFHeader* bhptr,char* fileName){
    BOFFILE bf = bof_read_open(fileName);
    BOFHeader bh = bof_read_header(bf);
    *bhptr = bh;

    for(int i = 0; i < bh.text_length/BYTES_PER_WORD; i++){
        bin_instr_t bi = instruction_read(bf);
        memory.instrs[i] = bi;
    }

    for(int i = 0; i < bh.data_length/BYTES_PER_WORD; i++){
        memory.words[bh.data_start_address + i] = bof_read_word(bf);
    }
}


const char *instructionCycle(bin_instr_t instr, int *PC, int *HI, int *LO, word_type* GPR){
     PC += 4;
     char *buf;
     char * name;
     unsigned result;
     instr_type it = instruction_type(instr);
     printf("%d\n", it);
     switch(it){
          case syscall_instr_type:
          break;
          case reg_instr_type:
          switch (instr.reg.func){
               
              case ADD_F:
                  GPR[instr.reg.rd] = GPR[instr.reg.rs] + GPR[instr.reg.rt];
              break;
              case SUB_F:
                  GPR[instr.reg.rd] = GPR[instr.reg.rs] - GPR[instr.reg.rt];
              break;
              case MUL_F:
                  result = (GPR[instr.reg.rs]) * (GPR[instr.reg.rt]);
                  HI = result >> 32;
                  LO = result & 0xFFFFFFFF;
              break;

              case DIV_F:
                  HI = GPR[instr.reg.rs] % GPR[instr.reg.rt];
                  LO = GPR[instr.reg.rs] / GPR[instr.reg.rt];
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
