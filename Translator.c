#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "Translator.h"
#include "Utilities.h"
#include "Parser.h"
#include "LangDefs.h"

#define INVALID_MACHINE_CODE_MASK 0x80000000
#define MIN_POSSIBLE_DATA_INT_VALUE -0x800000
#define MAX_POSSIBLE_DATA_INT_VALUE 0x800000-1
#define MIN_POSSIBLE_IM_INT_VALUE -0x100000
#define MAX_POSSIBLE_IM_INT_VALUE 0x100000-1
#define CPU_VALID_WORD_MASK 0xffffff

/**
* Invalid machine codes have bit 31 set.
*/
BOOLEAN is_valid_machine_code(int code)
{
    return (((unsigned int)code) & INVALID_MACHINE_CODE_MASK) != INVALID_MACHINE_CODE_MASK;
}

/**
* Builds machine code for string command. Assumes one operand with format: "string"
*/
int build_machine_code_for_string(char** operands, int** machineCodes)
{
    int i;
    size_t strLength = strlen(operands[0]) - 2;
    int* codeLines = (int*)malloc((strLength + 1) * sizeof(int));
    if (!codeLines)
    {
        return -1;
    }
    for (i = 1; i < strLength + 1; i++)
    {
        codeLines[i-1] = (int)operands[0][i];
    }
    codeLines[strLength] = 0;
    *machineCodes = codeLines;
    return (int)strLength + 1;
}

/**
* Builds machine code for data command. 
*/
int build_machine_code_for_data(char** operands, int operandsCnt, int** machineCodes)
{
    int i;
    int* codeLines = (int*)malloc(sizeof(int) * operandsCnt);
    if (!*codeLines)
    {
        return -1;
    }
    for (i = 0; i < operandsCnt; i++)
    {
        codeLines[i] = atoi(operands[i]);
        // the numbers range can be (-2^23)..(2^23-1)
        if (codeLines[i] < MIN_POSSIBLE_DATA_INT_VALUE || codeLines[i] > MAX_POSSIBLE_DATA_INT_VALUE)
        {
            free(codeLines);
            return -1;
        }
        codeLines[i] = codeLines[i] & CPU_VALID_WORD_MASK;
    }
    *machineCodes = codeLines;
    return operandsCnt;
}

/**
* Prepares command word and returns its value. 
*/
int prepare_command_word(int opcode, int funct, ADDRESS_RESOLUTION_TYPE srcAddrResType, int sourceRegNum, 
    ADDRESS_RESOLUTION_TYPE destAddrResType, int destRegNum)
{
    return  opcode << 18            |
            srcAddrResType << 16    |
            sourceRegNum << 13      |
            destAddrResType << 11   |
            destRegNum << 8         |
            funct << 3              |
            0x4;    // ARE - A always set for command.
}

int prepare_imidiate_value(int value)
{
    return CPU_VALID_WORD_MASK & (value << 3 | 0x4);
}

int init_invalid_machine_code()
{
    return INVALID_MACHINE_CODE_MASK;
}

/**
* Returns register number in the operand. 
* In case it is not applicable for the current command or operand is not register returns 0.
*/
int get_reg_num(int addrResBitMask, char* operand)
{
    if ((addrResBitMask & OP_TYPE_REGISTER) && is_register(operand))
    {
        return atoi(operand + 1);
    }
    return 0;
}

/**
* Returns address resolution type for the given command's address resolution bit mask and the operand.
*/
ADDRESS_RESOLUTION_TYPE get_addr_resolution_type(int addrResBitMask, char* operand)
{
    if ((addrResBitMask & OP_TYPE_REGISTER) && is_register(operand))
    {
        return ADDR_RES_TYPE_REGISTER;
    }
    if ((addrResBitMask & OP_TYPE_IMIDIATE) && is_imidiate(operand))
    {
        return ADDR_RES_TYPE_IMIDIATE;
    }
    if ((addrResBitMask & OP_TYPE_RELATIVE) && is_relative(operand))
    {
        return ADDR_RES_TYPE_RELATIVE;
    }
    return ADDR_RES_TYPE_DIRECT;
}

/**
* Initialize the payload code line with value.
*/
void init_payload_code_line(int* machineCodes, ADDRESS_RESOLUTION_TYPE addrResType, char* operand, int codeLineNum)
{
    if (addrResType == ADDR_RES_TYPE_IMIDIATE)
    {
        machineCodes[codeLineNum] = prepare_imidiate_value(atoi(operand + 1));
    }
    else if (addrResType == ADDR_RES_TYPE_DIRECT || addrResType == ADDR_RES_TYPE_RELATIVE)
    {
        machineCodes[codeLineNum] = init_invalid_machine_code();
    }
}

/**
* This function builds machine code for existing command wit cmdType.
*/
int build_machine_code_for_existing_cmd(CMD_TYPE* cmdType, char** operands, int** machineCodes)
{
    int* codeLines;
    int srcRegNum = 0, destRegNum = 0, nextOpNum = 1, curOperandIndex = 0;
    ADDRESS_RESOLUTION_TYPE srcAddrRes = ADDR_RES_TYPE_IMIDIATE, destAddrRes = ADDR_RES_TYPE_IMIDIATE;
    int machineCodesCnt = 1; // at least 1 for command
    BOOLEAN srcExist = cmdType->operands_number == 2;
    BOOLEAN destExist = cmdType->operands_number >= 1;
    
    if (srcExist)
    {
        srcRegNum = get_reg_num(cmdType->sourceOpType, operands[curOperandIndex]);
        srcAddrRes = get_addr_resolution_type(cmdType->sourceOpType, operands[curOperandIndex++]);
    }
    if (destExist)
    {
        destRegNum = get_reg_num(cmdType->destOpType, operands[curOperandIndex]);
        destAddrRes = get_addr_resolution_type(cmdType->destOpType, operands[curOperandIndex]);
    }

    if (srcExist && srcAddrRes != ADDR_RES_TYPE_REGISTER)
    {
        machineCodesCnt++;
    }
    if (destExist && destAddrRes != ADDR_RES_TYPE_REGISTER)
    {
        machineCodesCnt++;
    }

    codeLines = (int*)malloc(sizeof(int) * machineCodesCnt);
    if (!codeLines)
    {
        return -1;
    }
    codeLines[0] = prepare_command_word(cmdType->opcode, cmdType->funct, srcAddrRes, srcRegNum, destAddrRes, destRegNum);
    if (srcExist && srcAddrRes != ADDR_RES_TYPE_REGISTER)
    {
        init_payload_code_line(codeLines, srcAddrRes, operands[0], nextOpNum++);
    }
    if (destExist && destAddrRes != ADDR_RES_TYPE_REGISTER)
    {
        init_payload_code_line(codeLines, destAddrRes, operands[curOperandIndex], nextOpNum);
    }
    *machineCodes = codeLines;
    return machineCodesCnt;
}

int build_machine_code_for_cmd(char* cmd, char** operands, int** machineCodes)
{
    int i;
    
    for (i = 0; commands[i].name != NULL; i++)
    {
        if (strcmp(cmd, commands[i].name) == 0) // if command found
        {
            return build_machine_code_for_existing_cmd(&commands[i], operands, machineCodes);
        }
    }

    return -1;  // command not found
}

int build_machine_code(char* cmd, char** operands, int operandsCnt,  int** machineCodes)
{
    if (is_data_instr(cmd))
    {
        return build_machine_code_for_data(operands, operandsCnt, machineCodes);
    }
    else if (is_str_instr(cmd))
    {
        return build_machine_code_for_string(operands, machineCodes);
    }
    
    return build_machine_code_for_cmd(cmd, operands, machineCodes);
}

int build_machine_code_for_relative_label(int labelAddr, int cmdAddress)
{
    return (((labelAddr - cmdAddress) << 3) | 0x4) & CPU_VALID_WORD_MASK;
}

int build_machine_code_for_label(int labelAddress)
{
    return ((labelAddress << 3) | 0x2) & CPU_VALID_WORD_MASK;
}

int build_machine_code_for_external()
{
    return 0x1;
}
