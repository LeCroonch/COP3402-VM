//COP3402 Group 48 - Louis Au, Justin Rimmeli, Alejandro Fernandez
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

//function prototypes
void printData(int start, int end, word_type GPR[NUM_REGISTERS]);
void storeInstrs(BOFHeader* bh0, char* fileName);
void printElse(int PC, word_type GPR[NUM_REGISTERS], int HI, int LO);
void initGPR(BOFHeader bh, word_type GPR[NUM_REGISTERS]);
void instructionCycle(bin_instr_t instr, int *PC, int *HI, int *LO, word_type GPR[NUM_REGISTERS], int* trace);
void SystemHelper(bin_instr_t instr, word_type GPR[NUM_REGISTERS], int* trace);

int main(int argc, char * argv[]){
    int trace = 1; // 1 (Trace On) 0 (Trace Off)
     int PC = 0; // Program Counter
     BOFHeader bh;
     word_type GPR[NUM_REGISTERS]; // Registers
     int HI = 0; 
     int LO = 0;
 
     if(strcmp(argv[1], "-p") == 0){ //If -p present 

         storeInstrs(&bh ,argv[2]);

         printf("%4s %s\n", "Addr", "Instruction");
         for(int i = 0; i < bh.text_length/BYTES_PER_WORD; i++){
               printf("%4d %s\n", PC, instruction_assembly_form(memory.instrs[i]));
               PC = PC + 4;
         }
         initGPR(bh, GPR);
         printData(28, 29, GPR);

     } else { // Other cases

         storeInstrs(&bh, argv[1]);
         initGPR(bh, GPR);
         printElse(PC,GPR,HI,LO);
         
         while(1){ //Infinite loop of program cycle until Exit called
             instructionCycle(memory.instrs[PC/4], &PC, &HI, &LO, GPR, &trace);
             if(trace == 1)
             {
                 printElse(PC, GPR, HI, LO);
             }
         }
         printf("\n");
     }
}

void printData(int start, int end, word_type GPR[NUM_REGISTERS]){
    int newLine = 0;
    bool noDots = true;
    for(int i = GPR[start]; i <= GPR[end]; i += 4){
        if(memory.words[i] != 0){//check if not 0
            if(newLine % 5 == 0){//multiple of 5 print new line
                printf("\n");
            }
            printf("%8d: %-4d", i, memory.words[i]);
            newLine++;
            noDots = true;
        }else if(noDots){//if dots havnt been printed yet
            if (newLine % 5 == 0) {
                printf("\n");
            }
            printf("%8d: %-4d", i, memory.words[i]);
            printf("\t...");
            newLine++;
            noDots = false;
            if(start == 28)
            {
                break;
            }
        }else{

        }
    }
}

void storeInstrs(BOFHeader* bhptr, char* fileName){
    BOFFILE bf = bof_read_open(fileName);
    BOFHeader bh = bof_read_header(bf);
    *bhptr = bh;//header pointer
     
     //read instructions from the BOF file and store them in memory
    for(int i = 0; i < bh.text_length/BYTES_PER_WORD; i++){
        bin_instr_t bi = instruction_read(bf);
        memory.instrs[i] = bi;
    }

     //read data words from the BOF file and store them in memory
    for(int i = 0; i < bh.data_length; i +=4){
        memory.words[bh.data_start_address + i] = bof_read_word(bf);
    }
}

void printElse(int PC, word_type GPR[NUM_REGISTERS], int HI, int LO){
     //if HI and LO are greater than 0 print them, else print PC 
    if(HI > 0 || LO > 0)
    {
        printf("%8s: %d %8s: %d %8s: %d\n", "PC", PC, "HI", HI, "LO", LO);
    }else{
        printf("%8s: %d\n", "PC", PC);
    }

    for(int i = 0; i < 32; i++){
        if(i != 0 && i % 6 == 0){
            printf("\n");
        }
         //GPR entry and value
        printf("GPR[%-3s]: %-6d", regname_get(i), GPR[i]);
    }
    printf("\n");
    printData(28, 29, GPR);
    printData(29, 30, GPR);
    printf("\n==> addr:%5d %s\n", PC, instruction_assembly_form(memory.instrs[PC/4]));
}

void initGPR(BOFHeader bh, word_type GPR[NUM_REGISTERS]){
    for(int i = 0; i < 32; i++)
    {
        GPR[i] = 0;//initialize all GPRs to 0
    }
     //start and bottom addresses from BOF header
    GPR[28] = bh.data_start_address;
    GPR[29] = bh.stack_bottom_addr;
    GPR[30] = bh.stack_bottom_addr;
}

void instructionCycle(bin_instr_t instr, int *PC, int *HI, int *LO, word_type GPR[NUM_REGISTERS], int* trace){
   
    *PC += 4;
    int64_t result;
    instr_type it = instruction_type(instr);
    switch(it){
          case syscall_instr_type:
            switch (instr.syscall.code){
                case exit_sc:
                    exit(0);//exit
                break;
                case print_str_sc:
                    GPR[2] = printf("%s\n", &memory.bytes[GPR[4]]);//print string
                break;
                case print_char_sc:
                    GPR[2] = fputc(GPR[4], stdout);//print character
                break;
                case read_char_sc:
                    GPR[2] = getc(stdin);//read character
                break;
                case start_tracing_sc://start tracing
                    *trace = 1;
                break;
                case stop_tracing_sc://stop tracing
                    *trace = 0;
                break;
            }
          break;
          case reg_instr_type:
          switch (instr.reg.func){
              case ADD_F:
                  GPR[instr.reg.rd] = GPR[instr.reg.rs] + GPR[instr.reg.rt];//addition
              break;
              case SUB_F:
                  GPR[instr.reg.rd] = GPR[instr.reg.rs] - GPR[instr.reg.rt];//subtraction
              break;
              case MUL_F:
                  result = (GPR[instr.reg.rs]) * (GPR[instr.reg.rt]);//multiplication
                  *HI = result >> 32;//high bits of result
                  *LO = result & 0xFFFFFFFF;//low bits of result
              break;
              case DIV_F:
                   *HI = GPR[instr.reg.rs] % GPR[instr.reg.rt];//division remainder
                   *LO = GPR[instr.reg.rs] / GPR[instr.reg.rt];//divison quotient
              break;
              case MFHI_F:
                   GPR[instr.reg.rd] = *HI;
              break;
              case MFLO_F:
                   GPR[instr.reg.rd] = *LO;
              break;
              case AND_F:
                   GPR[instr.reg.rd] = (GPR[instr.reg.rs]) & (GPR[instr.reg.rt]);//bitwise AND
              break;
              case BOR_F:
                   GPR[instr.reg.rd] = ((GPR[instr.reg.rs]) | (GPR[instr.reg.rt]));//bitwise OR
              break;
              case NOR_F:
                   GPR[instr.reg.rd] = !((GPR[instr.reg.rs]) | (GPR[instr.reg.rt]));//bitwise NOR
              break;
              case XOR_F:
                   GPR[instr.reg.rd] = ((GPR[instr.reg.rs]) ^ (GPR[instr.reg.rt]));//bitwise XOR(exclusive OR)
              break;
              case SLL_F:
                   GPR[instr.reg.rd] = ((GPR[instr.reg.rt]) << (instr.reg.shift));//left shift
              break;
              case SRL_F:
                   GPR[instr.reg.rd] = ((GPR[instr.reg.rt]) >> (instr.reg.shift));//right shift
              break;
              case JR_F:
                   *PC = GPR[instr.reg.rs];//jump register
              break;
              case SYSCALL_F:
                SystemHelper(instr, GPR, trace);//function call
                }
                break;
          case immed_instr_type:
              switch (instr.immed.op){
                  case REG_O:
                      break;
                  case ADDI_O:
                    GPR[instr.immed.rt] = GPR[instr.immed.rs] + machine_types_sgnExt(instr.immed.immed);//add immediate
                  break;
                  case ANDI_O:
                    GPR[instr.immed.rt] = GPR[instr.immed.rs] & machine_types_zeroExt(instr.immed.immed);//bitwise AND w/ immediate
                      break;
                  case BORI_O:
                    GPR[instr.immed.rt] = GPR[instr.immed.rs] | machine_types_zeroExt(instr.immed.immed);//bitwise OR w/ immediate
                      break;
                  case XORI_O:
                    GPR[instr.immed.rt] = GPR[instr.immed.rs] ^ machine_types_zeroExt(instr.immed.immed);//bitwise XOR w/ immediate
                      break;
                  case BEQ_O:
                    if(GPR[instr.immed.rs] == GPR[instr.immed.rt]){
                        *PC = *PC + machine_types_formOffset(instr.immed.immed);//branch if equal
                    }
                    break;
                  case BGEZ_O:
                    if(GPR[instr.immed.rs] >= 0){
                        *PC = *PC + machine_types_formOffset(instr.immed.immed);//branch if >= 0
                    }
                    break;
                  case BGTZ_O:
                    if(GPR[instr.immed.rs] > 0){
                        *PC = *PC + machine_types_formOffset(instr.immed.immed);//branch if > 0
                    }
                    break;
                  case BLEZ_O:
                    if(GPR[instr.immed.rs] <= 0){
                        *PC = *PC + machine_types_formOffset(instr.immed.immed);//branch if <= 0
                    }
                    break;
                  case BLTZ_O:
                    if(GPR[instr.immed.rs] < 0){
                        *PC = *PC + machine_types_formOffset(instr.immed.immed);//branch if < 0
                    }
                    break;
                  case BNE_O:
                    if(GPR[instr.immed.rs] != GPR[instr.immed.rt]){
                        *PC = *PC + machine_types_formOffset(instr.immed.immed);//branch if not equal
                    }
                    break;
                  case LBU_O:
                    GPR[instr.immed.rt] = machine_types_zeroExt(memory.words[(GPR[instr.immed.rs] + machine_types_formOffset(instr.immed.immed))]);//load byte unsigned
                    break;
                  case LW_O:
                    GPR[instr.immed.rt] = memory.words[(GPR[instr.immed.rs] + machine_types_formOffset(instr.immed.immed))];//load word
                    break;
                  case SB_O:
                    memory.bytes[(GPR[instr.immed.rs] + machine_types_formOffset(instr.immed.immed))] = GPR[instr.immed.rt];//store byte
                    break;
                  case SW_O:
                    memory.words[(GPR[instr.immed.rs] + machine_types_formOffset(instr.immed.immed))] = GPR[instr.immed.rt];//store word
                    break;
              }
          break;
          case jump_instr_type://jump type instruction
              switch(instr.jump.op){
                case JMP_O:
                    *PC = machine_types_formAddress(*PC, instr.jump.addr);//PC updated to jump address
                    break;
                  case JAL_O:
                    GPR[31] = *PC;//return address saved at 31
                    *PC = machine_types_formAddress(*PC, instr.jump.addr);//PC updated to jump address
                    break;
              }
          break;
          default:
            bail_with_error("Unknown instruction type (%d) in instruction_assembly_form!", it);
          break;
        } 
         
    
    
}

void SystemHelper(bin_instr_t instr, word_type GPR[NUM_REGISTERS], int* trace){
    switch (instr.reg.op){
        case exit_sc:
            exit(0);//exit
        case print_str_sc:
            GPR[2] = printf("%s\n", &memory.bytes[GPR[4]]);//print string
        break;
        case print_char_sc:
            GPR[2] = fputc(GPR[4], stdout);//print character
        break;
        case read_char_sc:
            GPR[2] = getc(stdin);//read character
        break;
        case start_tracing_sc://start tracing
            *trace = 1;
        break;
        case stop_tracing_sc://stop tracing
            *trace = 0;
        break;
    }
}
