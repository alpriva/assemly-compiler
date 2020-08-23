#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "Parser.h"
#include "LangDefs.h"

/**
* The function checks if the cmdBuf has the supported command in it.
*/
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

/**
* The function checks if the cmdBuf has the supported directive in it.
*/
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

/**
* The function checks if the cmdBuf has the supported register in it.
*/
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
    // if first char is not alphabetical char or the label is a reserved keyword
    if ((isalpha(label[0]) == 0) || (is_keyword(label)))
    {
        return FALSE;
    }
    // pass on each char of the label starting from the second one.
    for (i = 1; i < length; i++)
    {
        // if the char is not alphabetical char and not a digit. return error.
        if ((isalpha(label[i]) == 0) && (isdigit(label[i]) == 0))
        {
            return FALSE;
        }
    }

    return TRUE;
}

BOOLEAN get_label(char *command_line, char **endp, char* label, BOOLEAN* foundLabel, int lineCnt)
{
    size_t labelLength;
    *foundLabel = FALSE;    // by default label is not found.

    if (sscanf(command_line, "%32s", label) == 1)
    {
        labelLength = strlen(label);    // including the ':' in the end.
        if (labelLength - 1 > MAX_LABEL_SIZE)
        {
            PRINT_ERR(lineCnt, "The label - %s is too long.", label);
            return FALSE;   // Indicate error
        }
        if (label[labelLength - 1] == ':')  // ends with ':' => label syntax
        {
            label[labelLength - 1] = '\0';    // replacing : with end of string.
            
            if (is_valid_label(label))  // check if label is valid
            {
                *endp = strchr(command_line, label[0]) + labelLength;
                *foundLabel = TRUE;
                return TRUE;
            }
            else
            {
                PRINT_ERR(lineCnt, "The label - %s is not valid.", label);
                return FALSE;   // Indicate error
            }
        }
        else
        {
            return TRUE;    // indicate success as label doesn't have to be there.
        }
    }
    return TRUE;   // indicate success as label doesn't have to be there.
}

BOOLEAN get_cmd(char *command_line, char **endp, char *commandBuffer, int lineCnt)
{
    if (sscanf(command_line, "%8s", commandBuffer) == 1)
    {
        if (is_cmd(commandBuffer) || is_directive(commandBuffer))
        {
            *endp = strchr(command_line, commandBuffer[0]) + strlen(commandBuffer);
            return TRUE;
        }
        else
        {
            if (commandBuffer[0] != '.')
            {
                PRINT_ERR(lineCnt, "Invalid command - %s.", commandBuffer);
            }
            else
            {
                PRINT_ERR(lineCnt, "Invalid directive - %s.", commandBuffer);
            }
        }
    }
    else
    {
        PRINT_ERR(lineCnt, "Missing command.");
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

BOOLEAN check_empty(char * commandLine)
{
    char buffer[LINE_SIZE];
    // if scanf fails to retreive a string, then the line is empty
    if (sscanf(commandLine, "%80s", buffer) != 1)
    {
        return TRUE;
    }

    return FALSE;
}

BOOLEAN is_comment_line(char * commandLine)
{
    char ch = ' ';
    // if scanf gets the first non white char from command line and it is ';' the it is a comment line.
    if (sscanf(commandLine, " %c", &ch) == 1 && ch == ';')
    {
        return TRUE;
    }
    return FALSE;
}

/**
* Retreives next operand from the given cmdLine. The operands are separated by ','. 
* this functions assumes that leading commas are removed prior to calling it.
* The last operand doesn't have the ',' in it.
*/
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
            if (buffer[i] == ',') // found comma in the buffer
            {
                buffer[i] = '\0'; // mark it with null terminator, so the buffer will hold the operand only.
                *endp = strchr(cmdLine, buffer[0]) + i; // increment the end pointer to the position after the operand and on the comma.
                return TRUE;
            }
        }
        
        // got here as no commas where in the buffer.
        *endp = strchr(cmdLine, buffer[0]) + strLen; // increment the end pointer to the position after the operand
        return TRUE;
    }

    return FALSE; // failed to get operand from the cmdLine.
}

/**
* Checks if given string represents valid supported integer.
*/
BOOLEAN check_int(char *str)
{
    int i;
    // valid integer can have leading '+'/'-' chars or its first char is a digit.
    if (str[0] == '+' || str[0] == '-' || isdigit(str[0]))
    {
        // checks the rest of characters they all have to be digits.
        for (i = 1; i < strlen(str); i++)
        {
            // if found non-digit char - return false.
            if (!isdigit(str[i]))
            {
                return FALSE;
            }
        }
        return TRUE;
    }

    // leading char didn't fit to the integer definition
    return FALSE;
}

/**
* Removes the first comma in cmdLine. If no comma was found return FALSE, otherwise TRUE.
*/
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

/**
* Prepares operands for .data instruction. On failure return -1.
* On success returns the number of operands that were parsed from the command line.
*/
int prepare_operands_for_data_instr(char *cmd, char* commandLine, char*** operands, int lineCnt)
{
    // buffer to hold the operand
    char buffer[LINE_SIZE];
    // intermediate variable to hold the statuses of different commands.
    BOOLEAN status;
    // pointer to the rest of command which is not yet parsed.
    char *restOfCmd = commandLine;
    // operandsCnt - how many operands are parsed for now.
    int operandsCnt = 0, i;
    // linked list to hold the operands as we parse them
    LinkedList *pList;
    // pointer to the operand in the linked list.
    Node *pCurrentNode;

    // initialize linked list
    pList = init_linked_list();
    RETURN_ON_MEMORY_FAILURE(pList, -1);

    while (TRUE)    // runs until all the operands are parsed. it is stopped when no more commas are in the command line.
    {
        operandsCnt++;  // working on next operand
        status = get_next_operand(restOfCmd, &restOfCmd, buffer); 
        GOTO_LABEL_ON_STATUS_ERR(status, prepare_for_exit, operandsCnt, -1, lineCnt, "Couldn't get the expected operand from the command line.");
        
        // check if operand is integer
        GOTO_LABEL_ON_STATUS_ERR(check_int(buffer), prepare_for_exit, operandsCnt, -1, lineCnt, "The operand is not integer.");
        
        status = add_entry_to_list(pList, buffer, strlen(buffer) + 1);
        GOTO_LABEL_ON_STATUS_ERR(status, prepare_for_exit, operandsCnt, -1, lineCnt, "Couldn't add operand.");
        
        if (!remove_comma(restOfCmd, &restOfCmd)) // no comma in case of finished parsing
        {
            GOTO_LABEL_ON_STATUS_ERR(check_empty(restOfCmd), prepare_for_exit, operandsCnt, -1, lineCnt, "The command has invalid syntax.");
            break;  // finished
        }
    }

    // Allocate operands array
    *operands = create_array_of_strings(operandsCnt);
    GOTO_LABEL_ON_NULL(*operands, prepare_for_exit, operandsCnt, -1, lineCnt, "No memory.");
    
    // copy operands from a list to array to return.
    pCurrentNode = pList->head;
    for (i = 0; i < operandsCnt; i++)
    {
        (*operands)[i] = malloc(strlen(pCurrentNode->data) + 1);
        if (!(*operands)[i])
        {
            PRINT_NO_MEMORY_ERR();
            free_array_of_strings(*operands, operandsCnt);
            break;
        }
        strcpy((*operands)[i], pCurrentNode->data);
        pCurrentNode = pCurrentNode->next;
    }

prepare_for_exit:
    free_linked_list(pList);
    return operandsCnt;
}

/**
* Prepares operand as label, in case it valid. On failure return -1.
* On success returns 1.
*/
int prepare_operand_as_label(char* commandLine, char*** operands, int lineCnt)
{
    char label[MAX_LABEL_SIZE];

    if (sscanf(commandLine, "%32s", label) == 1)
    {
        if (is_valid_label(label))
        {
            if (!check_empty(strchr(commandLine, label[0]) + strlen(label)))
            {
                PRINT_ERR(lineCnt, "Extra characters are in the end of the command.");
                return -1;
            }

            *operands = create_array_of_strings(1);
            RETURN_ON_MEMORY_FAILURE(*operands, -1);

            (*operands)[0] = malloc(strlen(label) + 1);
            if (!(*operands)[0])
            {
                PRINT_NO_MEMORY_ERR();
                free_array_of_strings(*operands, 1);
                return -1;
            }
            strcpy((*operands)[0], label);
            return 1;
        }
    }
    PRINT_ERR(lineCnt, "Couldn't find label in operand.");
    return -1;
}

/**
* Checks if the given string is string operand as defined in the assembler language.
*/
BOOLEAN is_string(char *str)
{
    int i;
    // check if string starts and ends with '"'
    if (!str || str[0] != '"' || str[strlen(str) - 1] != '"')
    {
        return FALSE;
    }
    
    // check that there is no '"' chars in the string.
    for (i = 1; i < strlen(str) - 1; i++)
    {
        if (str[i] == '"')
        {
            return FALSE;
        }
    }

    return TRUE;
}

/**
* Prepares operands for .string directive. On failure return -1.
* On success returns 1.
*/
int prepare_operands_for_str_instr(char *cmd, char* commandLine, char*** operands, int lineCnt)
{
    char *str;
    char *endp;
    char ch = ' ';
    if (sscanf(commandLine, " %c", &ch) != 1 || ch != '"')
    {
        RETURN_STATUS_ON_ERR(-1, lineCnt, "Expected operand should be a valid ANSI string, which should start and end with \".");
    }

    // get pointer to the start of the string.
    str = strchr(commandLine, '"');
    // get pointer to the end of the string.
    endp = strchr(str + 1, '"');
    if (!endp)
    {
        RETURN_STATUS_ON_ERR(-1, lineCnt, "Expected operand should be a valid ANSI string, which should start and end with \".");
    }

    if (!check_empty(endp + 1))
    {
        RETURN_STATUS_ON_ERR(-1, lineCnt, "Extra characters after command.");
    }

    *(endp + 1) = '\0';     
    *operands = create_array_of_strings(1);
    RETURN_ON_MEMORY_FAILURE(*operands, -1);
    (*operands)[0] = malloc(strlen(str) + 1);
    if (!(*operands)[0])
    {
        PRINT_NO_MEMORY_ERR();
        free_array_of_strings(*operands, 1);
        return -1;
    }
    strcpy((*operands)[0], str);
    return 1;
}

BOOLEAN is_imidiate(char *operand)
{
    // if the first char is not '#' the operand is not imidiate value.
    if (operand[0] != '#')
    {
        return FALSE;
    }
    return check_int(operand + 1);
}

/**
* It checks if the operand matches the direct address resolution syntax. Which is: it should be a label with valid syntax.
*/
BOOLEAN is_direct(char *operand)
{
    return is_valid_label(operand);
}

BOOLEAN is_relative(char *operand)
{
    // if the first char is not '&' the operand is not relative.
    if (operand[0] != '&')
    {
        return FALSE;
    }
    return is_valid_label(operand + 1); // the label should be valid.
}

/**
* Checks if the operand in the command pointed by pCmdType has the valid address resolution, based on its position.
*/
BOOLEAN is_valid_addr_resolution(CMD_TYPE *pCmdType, char *operand, int operandPosition, int lineCnt)
{
    int typeFlag = 0;
    char *positionDescription;  // position description is used for error printing.

    if (pCmdType->operands_number == 0)
    {
        PRINT_ERR(lineCnt, "No operands are expected for this command.");
        return FALSE;
    }

    // Set typeFlag and positionDescription
    if (pCmdType->operands_number == 1 && operandPosition == 0)
    {
        typeFlag = pCmdType->destOpType;
        positionDescription = "target"; 
    }
    else if (pCmdType->operands_number == 2 && operandPosition == 0)
    {
        typeFlag = pCmdType->sourceOpType;
        positionDescription = "source";
    }
    else if (pCmdType->operands_number == 2 && operandPosition == 1)
    {
        typeFlag = pCmdType->destOpType;
        positionDescription = "target";
    }
    else
    {
        return FALSE;
    }

    // 
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

    PRINT_ERR(lineCnt, "Invalid %s - %s.", positionDescription, operand);
    return FALSE;
}

/**
* Prepares operands for command instruction. In case of failure -1, in case of success the number of operands for this command.
*/
int prepare_operands_for_cmd_instr(char *command, char* commandLine, char*** operands, int lineCnt)
{
    // buffer to hold an operand
    char buffer[LINE_SIZE];
    // intermediate variable to hold statuses from different commands.
    BOOLEAN status = TRUE;
    int i, j;
    // hold how many operands are in the command.
    int operandsCnt = 0;

    for (i = 0; commands[i].name; i++)
    {
        if (strcmp(command, commands[i].name) == 0)     // if command found in the supported commands dictionary
        {
            if (commands[i].operands_number == 0)   // if the command doesn't have operands
            {
                if (!check_empty(commandLine))      // check that the rest of the command line is empty
                {
                    PRINT_ERR(lineCnt, "Extra characters after command - %s.", commandLine);
                    return -1;
                }
                return 0;   // return 0 operands for the command
            }

            operandsCnt = commands[i].operands_number;

            *operands = create_array_of_strings(operandsCnt);
            RETURN_ON_MEMORY_FAILURE(*operands, -1);

            // go through all the expected operands.
            for (j = 0; j < operandsCnt; j++)
            {
                status = get_next_operand(commandLine, &commandLine, buffer);
                GOTO_LABEL_ON_STATUS_ERR(status, prepare_to_return, status, FALSE, lineCnt, "Missing operand.");
                
                // does the operand matches the expected address resolution type.
                status = is_valid_addr_resolution(&commands[i], buffer, j, lineCnt);
                if (!status)
                {
                    goto prepare_to_return;
                }

                (*operands)[j] = (char *)malloc(strlen(buffer) + 1);
                if (!(*operands)[j])
                {
                    PRINT_NO_MEMORY_ERR();
                    status = FALSE;
                    goto prepare_to_return;
                }

                strcpy((*operands)[j], buffer);

                // No comma in case of finished parsing. No need to remove comma in case of last operand.
                if (j + 1 != operandsCnt && !remove_comma(commandLine, &commandLine)) 
                {
                    break;
                }
            }

            if (j < operandsCnt)    // break from the for loop above prematurely in case of missing operands.
            {
                PRINT_ERR(lineCnt, "Missing operands.");
                status = FALSE;
                goto prepare_to_return;
            }
            else if (!check_empty(commandLine))  // more characters in the command line
            {
                PRINT_ERR(lineCnt, "Extra characters after command - %s.", commandLine);
                status = FALSE;
                goto prepare_to_return;
            }
            break;  // break as the command is handled
        }
    }

prepare_to_return:
    if (!status)    // encountered error while getting the operands for a command
    {
        free_array_of_strings(*operands, operandsCnt);
        return -1;
    }
    return operandsCnt;
}

int prepare_operands_for_cmd(char* cmd, char* commandLine, char*** operands, int lineCnt)
{
    if (is_data_instr(cmd))
    {
        return prepare_operands_for_data_instr(cmd, commandLine, operands, lineCnt);
    }
    else if (is_str_instr(cmd))
    {
        return prepare_operands_for_str_instr(cmd, commandLine, operands, lineCnt);
    }
    else if (is_extern(cmd) || is_entry(cmd))
    {
        return prepare_operand_as_label(commandLine, operands, lineCnt);
    }
    else    // other commands
    {
        return prepare_operands_for_cmd_instr(cmd, commandLine, operands, lineCnt);
    }
}
