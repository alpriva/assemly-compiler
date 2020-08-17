#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "Parser.h"
#include "LangDefs.h"

BOOLEAN is_cmd(char *cmdBuf)
{
    int i;
    for (i = 0; commands[i].name != NULL; i++)
    {
        if (strcmp(cmdBuf, commands[i].name) == 0)
        {
            return TRUE;
        }
    }
    return FALSE;
}

BOOLEAN is_directive(char *cmdBuf)
{
    int i;
    for (i = 0; directives[i].name != NULL; i++)
    {
        if (strcmp(cmdBuf, directives[i].name) == 0)
        {
            return TRUE;
        }
    }
    return FALSE;
}

BOOLEAN is_register(char *cmdBuf)
{
    int i;
    for (i = 0; registers[i].reg != NULL; i++)
    {
        if (strcmp(cmdBuf, registers[i].reg) == 0)
        {
            return TRUE;
        }
    }
    return FALSE;
}

/**
* Cheks if given string is language's keyword.
* If string is a keyword returns TRUE, otherwise FALSE.
*/
BOOLEAN is_keyword(char* string)
{
    return is_cmd(string) || is_directive(string) || is_register(string);
}

/**
* Checks if given string is valid label.
* If label is valid returns TRUE, if not valid returns FALSE
*/
BOOLEAN is_valid_label(char *label)
{
    int i;
    size_t length = strlen(label);

    if ((isalpha(label[0]) == 0) || (label[length - 1] != ':') || (is_keyword(label)))
    {
        return FALSE;
    }
    for (i = 1; i < length - 1; i++)
    {
        if ((isalpha(label[i]) == 0) && (isdigit(label[i]) == 0))
        {
            return FALSE;
        }
    }

    return TRUE;
}

BOOLEAN get_label(char *command_line, char **endp, char* label)
{
    size_t labelLength;
    if (sscanf(command_line, "%32s", label) == 1)
    {
        if (is_valid_label(label))
        {
            labelLength = strlen(label);    // including the :
            label[labelLength - 1] = '\0';    // replacing : with end of string.
            *endp = strchr(command_line, label[0]) + labelLength;
            return TRUE;
        }
    }
    return FALSE;
}

BOOLEAN get_cmd(char *command_line, char **endp, char *commandBuffer)
{

    if (sscanf(command_line, "%7s", commandBuffer) == 1)
    {
        if (is_cmd(commandBuffer) || is_directive(commandBuffer))
        {
            *endp = strchr(command_line, commandBuffer[0]) + strlen(commandBuffer);
            return TRUE;
        }
    }
    return FALSE;
}

BOOLEAN is_data_instr(char *cmd)
{
    return strcmp(cmd, ".data") == 0;
}

BOOLEAN is_str_instr(char *cmd)
{
    return strcmp(cmd, ".string") == 0;
}

BOOLEAN is_data_cmd(char *cmd)
{
    return is_data_instr(cmd) || is_str_instr(cmd);
}

BOOLEAN is_extern(char * cmd)
{
    return strcmp(cmd, ".extern") == 0;
}

BOOLEAN is_entry(char * cmd)
{
    return strcmp(cmd, ".entry") == 0;
}

BOOLEAN is_valid_label_in_operand(char* label)
{
    int i;
    size_t length = strlen(label);

    if ((isalpha(label[0]) == 0) || (is_keyword(label)))
    {
        return FALSE;
    }
    for (i = 1; i < length; i++)
    {
        if ((isalpha(label[i]) == 0) && (isdigit(label[i]) == 0))
        {
            return FALSE;
        }
    }

    return TRUE;
}

BOOLEAN check_empty(char * commandLine)
{

    char buffer[LINE_SIZE];
    if (sscanf(commandLine, "%80s", buffer) != 1)
    {
        return TRUE;
    }
    return FALSE;
}

BOOLEAN is_comment_line(char * commandLine)
{
    char ch = ' ';
    if (sscanf(commandLine, " %c", &ch) == 1 && ch == ';')
    {
        return TRUE;
    }
    return FALSE;
}

BOOLEAN get_next_operand(char *cmdLine, char **endp, char *buffer)
{
    size_t strLen;
    int i;                                      
    if (sscanf(cmdLine, "%80s", buffer) == 1)   
    {
        if (buffer[0] == ',')
        {
            return FALSE;   // operand is expected
        }
        strLen = strlen(buffer);
        for (i = 0; i < strLen; i++)
        {
            if (buffer[i] == ',') // found comma
            {
                buffer[i] = '\0'; 
                *endp = strchr(cmdLine, buffer[0]) + i;
                return TRUE;
            }
        }
        *endp = strchr(cmdLine, buffer[0]) + strLen;
        return TRUE;
    }
    return FALSE;
}

BOOLEAN check_int(char *str)
{
    int i;
    if (str[0] == '+' || str[0] == '-' || isdigit(str[0]))
    {
        for (i = 1; i < strlen(str); i++)
        {
            if (!isdigit(str[i]))
            {
                return FALSE;
            }
        }
        return TRUE;
    }
    return FALSE;
}

BOOLEAN remove_comma(char *cmdLine, char **endp)
{
    char c = ' ';
    
    if (sscanf(cmdLine, " %c", &c) == 1 && c == ',')
    {
        *endp = strchr(cmdLine, ',') + 1; // point after the comma
        return TRUE;
    }
    return FALSE;
}

int prepare_operands_for_data_instr(char *cmd, char* commandLine, char*** operands)
{
    char buffer[LINE_SIZE];
    BOOLEAN status;
    char *restOfCmd = commandLine;
    int operandsCnt = 0, i;
    LinkedList *pList;
    Node *pCurrentNode;

    pList = init_linked_list();

    while (1)   // TODO: consider removing this while(1)
    {
        operandsCnt++;  // working on next operand
        status = get_next_operand(restOfCmd, &restOfCmd, buffer); 
        if (!status)
        {
            operandsCnt = -1;
            goto prepare_for_exit;
        }
        if (!check_int(buffer))
        {
            operandsCnt = -1;
            goto prepare_for_exit;
        }
        status = add_entry_to_list(pList, buffer, strlen(buffer) + 1);
        if (!status)
        {
            operandsCnt = -1;
            goto prepare_for_exit;
        }
        if (!remove_comma(restOfCmd, &restOfCmd)) // no comma in case of finished parsing
        {
            if (!check_empty(restOfCmd))
            {
                operandsCnt = -1;
                goto prepare_for_exit;
            }
            break;  // finished
        }
    }

    *operands = (char**)malloc(sizeof(char*) * operandsCnt);
    pCurrentNode = pList->head;
    for (i = 0; i < operandsCnt; i++)
    {
        (*operands)[i] = malloc(strlen(pCurrentNode->data) + 1);
        strcpy((*operands)[i], pCurrentNode->data);
        pCurrentNode = pCurrentNode->next;
    }

prepare_for_exit:
    free_linked_list(pList);
    return operandsCnt;
}

int prepare_operand_as_label(char* commandLine, char*** operands)
{
    char label[MAX_LABEL_SIZE];
    if (sscanf(commandLine, "%32s", label) == 1)
    {
        if (is_valid_label_in_operand(label))
        {
            if (!check_empty(strchr(commandLine, label[0]) + strlen(label)))
            {
                return -1;
            }
            *operands = (char**)malloc(sizeof(char*));
            (*operands)[0] = malloc(strlen(label) + 1);
            strcpy((*operands)[0], label);
            return 1;
        }
    }
    return -1;
}

BOOLEAN is_string(char *str)
{
    int i;
    if (!str || str[0] != '"' || str[strlen(str) - 1] != '"')
    {
        return FALSE;
    }
    for (i = 1; i < strlen(str) - 1; i++)
    {
        if (str[i] == '"')
        {
            return FALSE;
        }
    }
    return TRUE;
}

int prepare_operands_for_str_instr(char *cmd, char* commandLine, char*** operands)
{
    char buffer[LINE_SIZE];
    if (!get_next_operand(commandLine, &commandLine, buffer) || !check_empty(commandLine) || !is_string(buffer))
    {
        return -1;
    }
    
    *operands = (char**)malloc(sizeof(char*));
    (*operands)[0] = malloc(strlen(buffer) + 1);
    strcpy((*operands)[0], buffer);
    return 1;
}

BOOLEAN is_imidiate(char *operand)
{
    if (operand[0] != '#')
    {
        return FALSE;
    }
    return check_int(operand + 1);
}

BOOLEAN is_direct(char *operand)
{
    return is_valid_label_in_operand(operand);
}

BOOLEAN is_relative(char *operand)
{
    if (operand[0] != '&')
    {
        return FALSE;
    }
    return is_valid_label_in_operand(operand + 1);
}

BOOLEAN is_valid_cmd_operand(CMD_TYPE *pCmdType, char *operand, int operandPosition)
{
    int typeFlag = 0;
    if (pCmdType->operands_number == 0)
    {
        return FALSE;
    }
    if (pCmdType->operands_number == 1 && operandPosition == 0)
    {
        typeFlag = pCmdType->destOpType;
    }
    else if (pCmdType->operands_number == 2 && operandPosition == 0)
    {
        typeFlag = pCmdType->sourceOpType;
    }
    else if (pCmdType->operands_number == 2 && operandPosition == 1)
    {
        typeFlag = pCmdType->destOpType;
    }
    else
    {
        return FALSE;
    }
    if (typeFlag & OP_TYPE_IMIDIATE && is_imidiate(operand))  // if command can have first operand as imidiate and the operand is imidiate
    {
        return TRUE;
    } 
    if (typeFlag & OP_TYPE_DIRECT && is_direct(operand)) // if command can have first operand as direct and the operand is direct
    {
        return TRUE;
    }
    if (typeFlag & OP_TYPE_RELATIVE && is_relative(operand)) // if command can have first operand as relative and the operand is relative
    {
        return TRUE;
    }
    if (typeFlag & OP_TYPE_REGISTER && is_register(operand)) // if command can have first operand as register and the operand is register
    {
        return TRUE;
    }
    return FALSE;
}

int prepare_operands_for_cmd_instr(char *command, char* commandLine, char*** operands)
{
    char buffer[LINE_SIZE];
    BOOLEAN status;
    int i, j;
    int operandsCnt = 0;
    for (i = 0; commands[i].name; i++)
    {
        if (strcmp(command, commands[i].name) == 0)
        {
            if (commands[i].operands_number == 0)
            {
                return 0;
            }
            operandsCnt = commands[i].operands_number;
            *operands = (char**)malloc(operandsCnt * sizeof(char*));
            for (j = 0; j < operandsCnt; j++)
            {
                status = get_next_operand(commandLine, &commandLine, buffer);
                if (!status)
                {
                    free_array_of_strings(*operands, operandsCnt);
                    return -1;
                }
                if (!is_valid_cmd_operand(&commands[i], buffer, j))
                {
                    free_array_of_strings(*operands, operandsCnt);
                    return -1;
                }
                if (!remove_comma(commandLine, &commandLine)) // no comma in case of finished parsing
                {
                    if (!check_empty(commandLine))
                    {
                        free_array_of_strings(*operands, operandsCnt);
                        return -1;
                    }
                }
                (*operands)[j] = (char *)malloc(strlen(buffer) + 1);
                strcpy((*operands)[j], buffer);
            }
            break;
        }
    }

    return operandsCnt;
}

int prepare_operands_for_cmd(char* cmd, char* commandLine, char*** operands)
{
    if (is_data_instr(cmd))
    {
        return prepare_operands_for_data_instr(cmd, commandLine, operands);
    }
    else if (is_str_instr(cmd))
    {
        return prepare_operands_for_str_instr(cmd, commandLine, operands);
    }
    else if (is_extern(cmd) || is_entry(cmd))
    {
        return prepare_operand_as_label(commandLine, operands);
    }
    else    // other commands
    {
        return prepare_operands_for_cmd_instr(cmd, commandLine, operands);
    }
}