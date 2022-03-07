#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Utilities.h"
#include "Compiler.h"

/**
* The program compiles the provided assembly files.
* It supports multiple files provision.
* Only files with extension .as are supported.
* Only the file names shall be provided as arguments to this program.
* For each provided file - <FILE_NAME>, the <FILE_NAME>.ob will be generated.
* In case of applicable <FILE_NAME>.ext holding externals references details will be generated.
* In case of applicable <FILE_NAME>.ent holding entry labels details will be generated.
*/
int main(int argc, char *argv[])
{
    int i;
    FILE *fin;
    char *fileName;
    BOOLEAN status, foundError = FALSE;

    if (argc == 1)
    {
        printf("Error: there's no input file\n");
        return -1;
    }
    
    for (i = 1; i < argc; i++)
    {    
        fileName = concat(argv[i], ".as");
        fin = fopen(fileName, "r");
        free(fileName); // the fileName is not needed anymore.
        if (!fin)
        {
            // if wrong file was provided it is considered as fatal error.
            printf("Error: can't open %s file\n", fileName);
            return -1;
        }
        printf("Compiling %s.\n", argv[i]);
        status = compile(fin, argv[i]);
        if (!status) 
        {
            printf("Error: failed to compile file - %s\n", argv[i]);
            foundError = TRUE;
        }
        else
        {
            printf("%s compiled successfully.\n", argv[i]);
        }
        fclose(fin);
        
    }
    return foundError;
}

