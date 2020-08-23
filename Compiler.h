
#ifndef __COMPILER__
#define __COMPILER__

#include "Utilities.h"

/**
* Compiles the given source file - using the fin handle.
* In case of errors no output files are generated.
* It generates fileName.ob with machine code in hex format.
* In case .entry directives exist in the source file, it will generate fileName.ent file with entry details.
* In case .extern directives exist in the source file, it will generate fileName.ext file with externals details.
*/
BOOLEAN compile(FILE *fin, char *fileName);

#endif