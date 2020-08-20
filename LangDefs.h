#ifndef __LANGDEFS__
#define __LANGDEFS__

#define LINE_SIZE 80

#define COMMAND_SECTION_START 100
#define DATA_SECTION_START 0
#define MAX_LABEL_SIZE 31
#define MAX_CMD_SIZE 7

#define OP_TYPE_IMIDIATE 0x1
#define OP_TYPE_DIRECT   0x2
#define OP_TYPE_RELATIVE 0x4
#define OP_TYPE_REGISTER 0x8

typedef enum
{
    ADDR_RES_TYPE_IMIDIATE,
    ADDR_RES_TYPE_DIRECT,
    ADDR_RES_TYPE_RELATIVE,
    ADDR_RES_TYPE_REGISTER,
} ADDRESS_RESOLUTION_TYPE;

typedef struct
{
    char* name;
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
    char* reg;
} REGISTER_TYPE;

extern DIRECTIVE_TYPE directives[];

extern CMD_TYPE commands[];

extern REGISTER_TYPE registers[];

#endif