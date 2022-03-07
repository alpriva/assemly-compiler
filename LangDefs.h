/**
* This module holds all the assembler language definitions. 
*/

#ifndef __LANGDEFS__
#define __LANGDEFS__

// Maximum line size in the source file to be compiled.
#define LINE_SIZE 80

// The start of the command section.
#define COMMAND_SECTION_START 100
// The start of the data section.
#define DATA_SECTION_START 0
// Maximum length of a label
#define MAX_LABEL_SIZE 31
// Maximum length of a command
#define MAX_CMD_SIZE 7

// The defines below are used by the command address resolution type, which is a bit mask.
#define OP_TYPE_IMIDIATE 0x1
#define OP_TYPE_DIRECT   0x2
#define OP_TYPE_RELATIVE 0x4
#define OP_TYPE_REGISTER 0x8

// Address resolution type as it goes to the address resolution fields in compiled command.
typedef enum
{
    ADDR_RES_TYPE_IMIDIATE,
    ADDR_RES_TYPE_DIRECT,
    ADDR_RES_TYPE_RELATIVE,
    ADDR_RES_TYPE_REGISTER,
} ADDRESS_RESOLUTION_TYPE;

typedef struct
{
    char* name;     // directive description.
} DIRECTIVE_TYPE;

typedef struct
{
    char *name;     // the name of command
    int opcode;     // command opcode
    int funct;      // command funct
    int operands_number; // how many operands the command has
    int sourceOpType;   // source operand possible types - flags. each bit represents a type.
    int destOpType;     // destination operand possible types - flags. each bit represents a type.
} CMD_TYPE;

typedef struct
{
    char* reg;  // register description.
} REGISTER_TYPE;

// array which holds all the supported directives.
extern DIRECTIVE_TYPE directives[];

// array which holds all the supported commands.
extern CMD_TYPE commands[];

// array which holds all the supported registers.
extern REGISTER_TYPE registers[];

#endif