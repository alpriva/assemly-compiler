#include <stdlib.h>
#include "LangDefs.h"

DIRECTIVE_TYPE directives[] =
{
    { ".data" },
    { ".string" },
    { ".entry" },
    { ".extern" },
    { NULL },
};

CMD_TYPE commands[] = 
{
    { "mov", 0, 0, 2, OP_TYPE_IMIDIATE | OP_TYPE_DIRECT | OP_TYPE_REGISTER, OP_TYPE_DIRECT | OP_TYPE_REGISTER },
    { "cmp", 1, 0, 2, OP_TYPE_IMIDIATE | OP_TYPE_DIRECT | OP_TYPE_REGISTER, OP_TYPE_IMIDIATE | OP_TYPE_DIRECT | OP_TYPE_REGISTER },
    { "add", 2, 1, 2, OP_TYPE_IMIDIATE | OP_TYPE_DIRECT | OP_TYPE_REGISTER, 0xa },
    { "sub", 2, 2, 2, OP_TYPE_IMIDIATE | OP_TYPE_DIRECT | OP_TYPE_REGISTER, 0xa },
    { "lea", 4, 0, 2, OP_TYPE_DIRECT, OP_TYPE_DIRECT | OP_TYPE_REGISTER },
    { "clr", 5, 1, 1, 0x0, OP_TYPE_DIRECT | OP_TYPE_REGISTER },
    { "not", 5, 2, 1, 0x0, OP_TYPE_DIRECT | OP_TYPE_REGISTER },
    { "inc", 5, 3, 1, 0x0, OP_TYPE_DIRECT | OP_TYPE_REGISTER },
    { "dec", 5, 4, 1, 0x0, OP_TYPE_DIRECT | OP_TYPE_REGISTER },
    { "jmp", 9, 1, 1, 0x0, OP_TYPE_DIRECT | OP_TYPE_RELATIVE },
    { "bne", 9, 2, 1, 0x0, OP_TYPE_DIRECT | OP_TYPE_RELATIVE },
    { "jsr", 9, 3, 1, 0x0, OP_TYPE_DIRECT | OP_TYPE_RELATIVE },
    { "red", 12, 0, 1, 0x0, OP_TYPE_DIRECT | OP_TYPE_REGISTER },
    { "prn", 13, 0, 1, 0x0, OP_TYPE_IMIDIATE | OP_TYPE_DIRECT | OP_TYPE_REGISTER },
    { "rts", 14, 0, 0, 0x0, 0x0 },
    { "stop", 15, 0, 0, 0x0, 0x0 },
    { NULL }
};

REGISTER_TYPE registers[] = 
{
    { "r0" },
    { "r1" },
    { "r2" },
    { "r3" },
    { "r4" },
    { "r5" },
    { "r6" },
    { "r7" },
    { NULL }
};
