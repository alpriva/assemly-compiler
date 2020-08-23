#ifndef __TRANSLATOR__
#define __TRANSLATOR__

#include "Utilities.h"
/**
* Returns if the given word is valid machine code.
*/
BOOLEAN is_valid_machine_code(int code);

/**
* Builds machine code for a command using the given operands.
* It assumes that operands array is populated correctly according to the cmd. 
* It populates the machine codes array and puts it to the value pointed by the machineCodes.
* In case of operands are labels, it leaves the machine code invalid for this operand.
* Returns the number of words in the machineCodes array.
*/
int build_machine_code(char* cmd, char** operands, int operandsCnt, int** machineCodes, int lineCnt);

/**
* Builds and returns the machine code for relative to cmdAddress label.
*/
int build_machine_code_for_relative_label(int labelAddr, int cmdAddress);

/**
* Builds and returns the machine code for label operand.
*/
int build_machine_code_for_label(int labelAddress);

/**
* Builds machine code for external label reference
*/
int build_machine_code_for_external();

#endif