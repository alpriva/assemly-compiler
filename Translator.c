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
#define STRING_NULL_TERMINATOR_VAL 0 

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
    size_t strLength = strlen(operands[0]) - 2; // 2 for '"' at the start and the end of the operand
    int* codeLines = (int*)malloc((strLength + 1) * sizeof(int));
    RETURN_ON_MEMORY_FAILURE(codeLines, -1);

    for (i = 1; i < strLength + 1; i++)
    {
        codeLines[i-1] = (int)operands[0][i];
    }
    codeLines[strLength] = STRING_NULL_TERMINATOR_VAL;   // the last word should be null terminator
    *machineCodes = codeLines;
    return (int)strLength + 1;
}

/**
* Builds machine code for data command. 
*/
int build_machine_code_for_data(char** operands, int operandsCnt, int** machineCodes, int lineCnt)
{
    int i;
    int* codeLines = (int*)malloc(sizeof(int) * operandsCnt);
    RETURN_ON_MEMORY_FAILURE(codeLines, -1);

    for (i = 0; i < operandsCnt; i++)
    {
        codeLines[i] = atoi(operands[i]);
        // the numbers range can be (-2^23)..(2^23-1)
        if (codeLines[i] < MIN_POSSIBLE_DATA_INT_VALUE || codeLines[i] > MAX_POSSIBLE_DATA_INT_VALUE)
        {
            PRINT_ERR(lineCnt, "Operand %d doesn't fit into 24 bits.", codeLines[i]);
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

/**
* Prepare imidiate value translation to machine code.
*/
int prepare_imidiate_value(int value)
{
    return CPU_VALID_WORD_MASK & (value << 3 | 0x4);    // ARE - always set for immidiate value.
}

/**
* Returns not valid machine code.
*/
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
BOOLEAN init_payload_code_line(int* machineCodes, ADDRESS_RESOLUTION_TYPE addrResType, char* operand, int codeLineNum)
{
    int imidiateVal;
    if (addrResType == ADDR_RES_TYPE_IMIDIATE)
    {
        imidiateVal = atoi(operand + 1);
        if (imidiateVal < MIN_POSSIBLE_IM_INT_VALUE || imidiateVal > MAX_POSSIBLE_IM_INT_VALUE)
        {
            return FALSE;
        }
        machineCodes[codeLineNum] = prepare_imidiate_value(imidiateVal);
    }
    else if (addrResType == ADDR_RES_TYPE_DIRECT || addrResType == ADDR_RES_TYPE_RELATIVE)
    {
        machineCodes[codeLineNum] = init_invalid_machine_code();
    }

    return TRUE;
}

/**
* This function builds machine code for existing command wit cmdType.
*/
int build_machine_code_for_existing_cmd(CMD_TYPE* cmdType, char** operands, int** machineCodes, int lineCnt)
{
    int* codeLines;
    int srcRegNum = 0, destRegNum = 0, nextOpNum = 1, curOperandIndex = 0;
    ADDRESS_RESOLUTION_TYPE srcAddrRes = ADDR_RES_TYPE_IMIDIATE, destAddrRes = ADDR_RES_TYPE_IMIDIATE;
    int machineCodesCnt = 1; // at least 1 for command
    BOOLEAN srcExist = cmdType->operands_number == 2;
    BOOLEAN destExist = cmdType->operands_number >= 1;
    BOOLEAN status = TRUE;
    
    if (srcExist)
    {
        // assigns source reg in case of not valid stays 0.
        srcRegNum = get_reg_num(cmdType->sourceOpType, operands[curOperandIndex]);
        // assigns source address resolution type.
        srcAddrRes = get_addr_resolution_type(cmdType->sourceOpType, operands[curOperandIndex++]);
    }
    if (destExist)
    {
        // assigns destination reg in case of not valid stays 0.
        destRegNum = get_reg_num(cmdType->destOpType, operands[curOperandIndex]);
        // assigns destination address resolution type.
        destAddrRes = get_addr_resolution_type(cmdType->destOpType, operands[curOperandIndex]);
    }

    if (srcExist && srcAddrRes != ADDR_RES_TYPE_REGISTER)
    {
        machineCodesCnt++;  // In case the source exists and it is not register. Need to allocate another word for it.
    }
    if (destExist && destAddrRes != ADDR_RES_TYPE_REGISTER)
    {
        machineCodesCnt++; // In case the destination exists and it is not register. Need to allocate another word for it.
    }

    codeLines = (int*)malloc(sizeof(int) * machineCodesCnt);
    RETURN_ON_MEMORY_FAILURE(codeLines, -1);
    
    codeLines[0] = prepare_command_word(cmdType->opcode, cmdType->funct, srcAddrRes, srcRegNum, destAddrRes, destRegNum);
    if (srcExist && srcAddrRes != ADDR_RES_TYPE_REGISTER)
    {
        // In case of source exists and it is not register, initialize the payload word for it.
        status = init_payload_code_line(codeLines, srcAddrRes, operands[0], nextOpNum++);
        GOTO_LABEL_ON_STATUS_ERR(status, prepare_to_return, machineCodesCnt, -1, lineCnt, "The source operand doesn't fit.");
    }
    if (destExist && destAddrRes != ADDR_RES_TYPE_REGISTER)
    {
        // In case of destination exists and it is not register, initialize the payload word for it.
        status = init_payload_code_line(codeLines, destAddrRes, operands[curOperandIndex], nextOpNum);
        GOTO_LABEL_ON_STATUS_ERR(status, prepare_to_return, machineCodesCnt, -1, lineCnt, "The target operand doesn't fit.");
    }
prepare_to_return:
    if (!status)
    {
        free(codeLines);
    }
    else
    {
        *machineCodes = codeLines;
    }
    return machineCodesCnt;
}

/**
* Builds machine code for instruction.
*/
int build_machine_code_for_cmd(char* cmd, char** operands, int** machineCodes, int lineCnt)
{
    int i;
    
    for (i = 0; commands[i].name != NULL; i++)
    {
        if (strcmp(cmd, commands[i].name) == 0) // if command found
        {
            return build_machine_code_for_existing_cmd(&commands[i], operands, machineCodes, lineCnt);
        }
    }
    PRINT_ERR(lineCnt, "Command - %s is not supported.", cmd);
    return -1;  // command not found
}

int build_machine_code(char* cmd, char** operands, int operandsCnt,  int** machineCodes, int lineCnt)
{
    if (is_data_instr(cmd))
    {
        return build_machine_code_for_data(operands, operandsCnt, machineCodes, lineCnt);
    }
    else if (is_str_instr(cmd))
    {
        return build_machine_code_for_string(operands, machineCodes);
    }
    
    return build_machine_code_for_cmd(cmd, operands, machineCodes, lineCnt);
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

