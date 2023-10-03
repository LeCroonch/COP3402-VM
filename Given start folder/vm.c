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

void printData(BOFHeader* bh);
void storeInstrs(BOFHeader* bh0,char* fileName);
void printElse(int* PC, BOFHeader* bh, word_type GPR[NUM_REGISTERS]);
void initGPR(BOFHeader* bh, word_type GPR[NUM_REGISTERS]);
void instructionCycle(bin_instr_t instr, int *PC, int *HI, int *LO, word_type GPR[NUM_REGISTERS]);

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
         initGPR(&bh, GPR);

         for(int i = 0; i < bh.text_length/BYTES_PER_WORD; i++){
             printElse(&PC, &bh, GPR);
             instructionCycle(memory.instrs[i], &PC, &HI, &LO, GPR);
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

void printElse(int* PC, BOFHeader* bh, word_type GPR[NUM_REGISTERS]){
    printf("%8s: %d\n", "PC", *PC);
    for(int i = 0; i < 32; i++){
        if(i != 0 && i % 6 == 0){
            printf("\n");
        }
        printf("GPR[%-3s]: %-6d", regname_get(i), GPR[i]);
    }
    printf("\n");
    printData(bh);
    printf("==> addr: %-5u: %s\n", *PC, instruction_assembly_form(memory.instrs[(*PC % 4)]));
}

void initGPR(BOFHeader* bh, word_type GPR[NUM_REGISTERS]){
    GPR[28] = bh->data_start_address;
    GPR[29] = bh->stack_bottom_addr;
    GPR[30] = bh->stack_bottom_addr;
}

void instructionCycle(bin_instr_t instr, int *PC, int *HI, int *LO, word_type GPR[NUM_REGISTERS]){
     *PC += 4;
     int64_t result;
     instr_type it = instruction_type(instr);
     printf("%d\n", it);
     switch(it){
          case syscall_instr_type:
            switch (instr.syscall.code){

                case exit_sc:
                    exit(0);
                break;
                case print_str_sc:
                    
                break;
                case print_char_sc:
                    GPR[2] = fputc(GPR[4], stdout);
                break;
                case read_char_sc:
                    GPR[2] = getc(stdin);
                break;
                case start_tracing_sc:

                break;
                case stop_tracing_sc:
                
                break;

            }
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
                  *HI = result >> 32;
                  *LO = result & 0xFFFFFFFF;
              break;

              case DIV_F:
                   *HI = GPR[instr.reg.rs] % GPR[instr.reg.rt];
                   *LO = GPR[instr.reg.rs] / GPR[instr.reg.rt];
              break;
              case MFHI_F:
                   GPR[instr.reg.rd] = *HI;
              break;
              case MFLO_F:
                   GPR[instr.reg.rd] = *LO;
              break;
              case AND_F:
                   GPR[instr.reg.rd] = (GPR[instr.reg.rs]) && (GPR[instr.reg.rt]);
              break;
              case BOR_F:
                   GPR[instr.reg.rd] = ((GPR[instr.reg.rs]) || (GPR[instr.reg.rt]));
              break;
              case NOR_F:
                   GPR[instr.reg.rd] = !((GPR[instr.reg.rs]) || (GPR[instr.reg.rt]));
              break;
              case XOR_F:
                   GPR[instr.reg.rd] = ((GPR[instr.reg.rs]) ^ (GPR[instr.reg.rt])); 
              break;
              case SLL_F:
                   GPR[instr.reg.rd] = ((GPR[instr.reg.rt]) << (GPR[instr.reg.shift]));
              break;
              case SRL_F:
                   GPR[instr.reg.rd] = ((GPR[instr.reg.rt]) >> (GPR[instr.reg.shift]));
              break;
              case JR_F:
                   *PC = GPR[instr.reg.rs];
              break;
              case SYSCALL_F:
                   switch (instr.reg.op){
                        case exit_sc:
                        exit(0);
                        break;
                        case print_str_sc:

                        break;
                        case print_char_sc:

                        break;
                        case read_char_sc:

                        break;
                        case start_tracing_sc:

                        break;
                        case stop_tracing_sc:

                        break;
                   }
              break;

          }
          case immed_instr_type:
              switch (instr.immed.op){
                  case REG_O:
                      break;;
                  case ADDI_O:
                    GPR[instr.immed.rt] = GPR[instr.immed.rs] + machine_types_sgnExt(instr.immed.immed);
                  break;
                  case ANDI_O:
                    GPR[instr.immed.rt] = GPR[instr.immed.rs] & machine_types_zeroExt(instr.immed.immed);
                      break;
                  case BORI_O:
                    GPR[instr.immed.rt] = GPR[instr.immed.rs] | machine_types_zeroExt(instr.immed.immed);
                      break;
                  case XORI_O:
                    GPR[instr.immed.rt] = GPR[instr.immed.rs] ^ machine_types_zeroExt(instr.immed.immed);
                      break;
                  case BEQ_O:
                    if(GPR[instr.immed.rs] == GPR[instr.immed.rt]){
                        *PC = *PC + machine_types_formOffset(instr.immed.immed);
                    }
                      break;
                  case BGEZ_O:
                    if(GPR[instr.immed.rs] >= 0){
                        *PC = *PC + machine_types_formOffset(instr.immed.immed);
                    }
                      break;
                  case BGTZ_O:
                    if(GPR[instr.immed.rs] > 0){
                        *PC = *PC + machine_types_formOffset(instr.immed.immed);
                    }
                      break;
                  case BLEZ_O:
                    if(GPR[instr.immed.rs] <= 0){
                        *PC = *PC + machine_types_formOffset(instr.immed.immed);
                    }
                      break;
                  case BLTZ_O:
                    if(GPR[instr.immed.rs] < 0){
                        *PC = *PC + machine_types_formOffset(instr.immed.immed);
                    }
                      break;
                  case BNE_O:
                    if(GPR[instr.immed.rs] != GPR[instr.immed.rt]){
                        *PC = *PC + machine_types_formOffset(instr.immed.immed);
                    }
                      break;
                  case LBU_O:
                    GPR[instr.immed.rt] = machine_types_zeroExt(memory.bytes[GPR[instr.immed.rs] + machine_types_formOffset(instr.immed.immed)]);
                      break;
                  case LW_O:
                    GPR[instr.immed.rt] = memory.words[GPR[instr.immed.rs] + machine_types_formOffset(instr.immed.immed)];
                      break;
                  case SB_O:
                    memory.bytes[GPR[instr.immed.rs] + machine_types_formOffset(instr.immed.immed)] = GPR[instr.immed.rt];
                      break;
                  case SW_O:
                    memory.words[GPR[instr.immed.rs] + machine_types_formOffset(instr.immed.immed)] = GPR[instr.immed.rt];
                      break;
              }
          break;
          case jump_instr_type:
              switch(instr.jump.op){
                  case JMP_O:
                    *PC = machine_types_formAddress(*PC, instr.jump.addr);
                      break;
                  case JAL_O:
                    GPR[31] = *PC;
                    *PC = machine_types_formAddress(*PC, instr.jump.addr);
                      break;
              }
          break;
          default:
          bail_with_error("Unknown instruction type (%d) in instruction_assembly_form!", it);
     }
}
