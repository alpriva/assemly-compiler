
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Utilities.h"
#include "Compiler.h"

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
        if (!fin)
        {
            printf("Error: can't open %s file\n", fileName);
            free(fileName);
            return -1;
        }
        free(fileName);
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
        
    }
    return foundError;
}

