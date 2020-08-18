#ifndef __PARSER__
#define __PARSER__

#include "Utilities.h"

/**
* Retreives label from the command line. The function assigns the tail to the *endp.
* In case label not found returns FALSE and *endp is not moved.
*/
BOOLEAN get_label(char *command_line, char **endp, char* label);

/**
* Retreives command from the command line. In case if not found or command unknown it FALSE.
* The function assigns the tail to the *endp. the command is store in the commandBuffer.
*/
BOOLEAN get_cmd(char *command_line, char **endp, char *commandBuffer, int lineCnt);

/**
* Cheks if command is data command. Returns TRUE if
* command is data or string instruction, returns FALSE otherwise
*/
BOOLEAN is_data_cmd(char *cmd);

/**
* Cheks if command is extern instruction. Returns TRUE if
* command is extern instruction, returns FALSE otherwise.
*/
BOOLEAN is_extern(char *cmd);

/**
* Cheks if command is entry instruction. Returns TRUE if
* command is extern instruction, returns FALSE otherwise.
*/
BOOLEAN is_entry(char *cmd);

/**
* Checks that command line is empty
*/
BOOLEAN check_empty(char *commandLine);

/**
* Checks that command line is comment
*/
BOOLEAN is_comment_line(char * commandLine);

/**
* Prepares operands of the given command into the given operands array.
* The number of operands and the format depends on the given command.
* it returns the number of operands added to the operands array.
* In case of parsing error or some other failure return -1.
*/
int prepare_operands_for_cmd(char* cmd, char* command_line, char*** operands, int lineCnt);

/**
* Checks if the command is string.
*/
BOOLEAN is_str_instr(char* cmd);

/**
* Checks if the command is data.
*/
BOOLEAN is_data_instr(char* cmd);

/**
* Checks if the given operand denotes a valid register
*/
BOOLEAN is_register(char* operand);

/**
* Checks if the given operand is imidiate value
*/
BOOLEAN is_imidiate(char *operand);

/**
* Checks if the given operand is relative
*/
BOOLEAN is_relative(char *operand);

#endif