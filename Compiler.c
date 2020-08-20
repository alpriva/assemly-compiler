#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Compiler.h"
#include "Parser.h"
#include "Tables.h"
#include "Translator.h"
#include "LangDefs.h"

// the buffer should include one more character and the null terminator
#define LABEL_BUFFER_SIZE MAX_LABEL_SIZE + 2
// the buffer should include one more character and the null terminator
#define CMD_BUFFER_SIZE MAX_CMD_SIZE + 2
// the buffer should include one more character and the null terminator
#define LINE_BUFFER_SIZE LINE_SIZE + 2

/**
* This function handles the entry command. Assumes that Symbol table is already populated.
* It searches for symbol corresponding to entry command's operand. If found, modifies the symbol table to note that 
* the symbol is entry. If not found - error.
* It returns FALSE on any error it finds.
* Otherwise TRUE.
*/
BOOLEAN handle_entry_cmd(char *cmd, char *commandLine, int lineCnt, SYMBOL_TABLE *pSymbolTable)
{
    SYMBOL_TABLE_ENTRY *pSymbolTableEntry = NULL;
    int operandsCnt;
    BOOLEAN status = TRUE;
    char **operands;

    operandsCnt = prepare_operands_for_cmd(cmd, commandLine, &operands, lineCnt);
    if (operandsCnt < 0)
    {
        return FALSE;
    }

    pSymbolTableEntry = get_symbol_entry(pSymbolTable, operands[0]);
    if (!pSymbolTableEntry)
    {
        printf("Error: In line - %d. No such symbol - %s\n", lineCnt, operands[0]);    // TODO convert to usage of variadic macro 
        status = FALSE;
    }
    else if (pSymbolTableEntry->type & EXTERNAL)
    {
        PRINT_ERR(lineCnt, "The external label in the current file cannot be entry.");
        status = FALSE;
    }
    else
    {
        pSymbolTableEntry->type |= ENTRY;
    }
    free_array_of_strings(operands, operandsCnt);
    return status;
}

/**
* Retreives the label by the index from the operands array.
* e.g. index = 0 - first label in the operands array.
* index = 1 - second label in the operands array.
*/
char* get_label_from_operands(char** operands, int operandsCnt, int index)
{
    int i;
    int labelIndex = 0;
    char *label = NULL;
    for (i = 0; i < operandsCnt; i++)
    {
        if (!is_register(operands[i]) && !is_imidiate(operands[i]))
        {
            label = operands[i];
            if (labelIndex == index)
            {
                break;
            }
            labelIndex++;
        }
    }

    return label;
}

/**
* This function, completes the command with index cmdCnt in the Cmd table. It assumes that symbol table is populated.
* In case of failure return FALSE, else TRUE.
*/
BOOLEAN complete_cmd_in_cmd_table(CMD_TABLE* pCmdTable, SYMBOL_TABLE* pSymbolTable, EXTERNALS_TABLE* pExternalTable, char* cmd, char* commandLine, int cmdCnt, int lineCnt)
{
    CMD_TABLE_ENTRY *pCmdTableEntry;
    SYMBOL_TABLE_ENTRY *pSymbolTableEntry;
    int i, operandsCnt;
    char *label = NULL;
    char **operands = NULL;
    BOOLEAN status = TRUE;
    pCmdTableEntry = get_cmd_table_entry(pCmdTable, cmdCnt);
    if (!pCmdTableEntry)
    {
        PRINT_ERR(lineCnt, "The command is unexpected");
        return FALSE;
    }
    operandsCnt = prepare_operands_for_cmd(cmd, commandLine, &operands, lineCnt);
    if (operandsCnt < 0)
    {
        printf("Error: In line - %d. Couldn't parse operands for command - %s\n.", lineCnt, cmd); // TODO convert to usage of variadic macro 
        return FALSE;
    }
    for (i = 1; i < pCmdTableEntry->MachineCodesLength; i++)    // starting from the second word, as the first is always a command
    {
        if (!is_valid_machine_code(*(pCmdTableEntry->MachineCodes + i)))    // the machine code is not valid only in case of a label
        {
            label = get_label_from_operands(operands, operandsCnt, i - 1);
            if (!label)
            {
                printf("Error: In line - %d. Label not found in operands of the command: %s\n.", lineCnt, cmd); // TODO convert to usage of variadic macro 
                status = FALSE;
                goto prepare_for_exit;
            }
            if (is_relative(label))
            {
                pSymbolTableEntry = get_symbol_entry(pSymbolTable, label + 1);
            }
            else
            {
                pSymbolTableEntry = get_symbol_entry(pSymbolTable, label);
            }
            
            if (!pSymbolTableEntry)
            {
                printf("Error: In line - %d. No such symbol - %s\n", lineCnt, label); // TODO convert to usage of variadic macro 
                status = FALSE;
                goto prepare_for_exit;
            }

            if (pSymbolTableEntry->type & EXTERNAL && is_relative(label))
            {
                printf("Error: In line - %d. Invalid operand - %s\n", lineCnt, label); // TODO convert to usage of variadic macro 
                status = FALSE;
                goto prepare_for_exit;
            }

            if (is_relative(label))
            {
                *(pCmdTableEntry->MachineCodes + i) = build_machine_code_for_relative_label(pSymbolTableEntry->addr, pCmdTableEntry->cmdAddress);
            }
            else if (pSymbolTableEntry->type & EXTERNAL)
            {
                *(pCmdTableEntry->MachineCodes + i) = build_machine_code_for_external();
                if (!add_call_addr_to_external(pExternalTable, pSymbolTableEntry->label, pCmdTableEntry->cmdAddress + i))
                {
                    printf("Error: In line - %d. Symbol %s is undefined in the externals list", lineCnt, label); // TODO convert to usage of variadic macro 
                    status = FALSE;
                    goto prepare_for_exit;
                }
            }
            else 
            {
                *(pCmdTableEntry->MachineCodes + i) = build_machine_code_for_label(pSymbolTableEntry->addr);
            }
        }
    }

prepare_for_exit:
    free_array_of_strings(operands, operandsCnt);

    return status;
}

void print_array_of_strings(char **arr, int count, int lineCnt)
{
    int i;
    printf("Array of strings at line %d: ", lineCnt);
    for (i = 0; i < count; i++)
    {
        printf("%s; ", arr[i]);
    }
    printf("\n");
}

void add_end_of_instructions_to_data_symbols(SYMBOL_TABLE_ENTRY* symbolEntry, void* additionalInfo)
{
    int icf = *((int*)additionalInfo);
    if (symbolEntry->type & DATA)
    {
        symbolEntry->addr += icf;
    }
}

void complete_symbols_addresses_for_data(SYMBOL_TABLE* pSymbolTable, int icf)
{
    apply_to_symbol_table_entries(pSymbolTable, add_end_of_instructions_to_data_symbols, &icf);
}

BOOLEAN add_label_to_symbol_table(SYMBOL_TABLE* pSymbolTable, char* label, SYMBOL_TYPE labelType, int addr, int lineCnt)
{
    SYMBOL_TABLE_ENTRY *pSymbol;
    
    pSymbol = get_symbol_entry(pSymbolTable, label);
    if(pSymbol)
    {
        PRINT_ERR(lineCnt, "The label is already defined.");
        return FALSE;
    }
    return add_entry_to_symbol_table(pSymbolTable, label, labelType, addr);
}

/**
* Performs the first pass of the compiler algorithm.
* Returns TRUE if successefull, otherwise FALSE.
*/
BOOLEAN first_pass(FILE* fin, SYMBOL_TABLE* pSymbolTable, CMD_TABLE* pCmdTable, DATA_TABLE* pDataTable, EXTERNALS_TABLE* pExternalsTable, int *icf, int *dcf)
{
    char cmdLine[LINE_BUFFER_SIZE];
    int dc = DATA_SECTION_START, ic = COMMAND_SECTION_START;
    int lineCnt = 0, machineCodesLength, operandsCnt;
    BOOLEAN found_error = FALSE, status, found_label, lineWasTooBig = FALSE;
    char label[LABEL_BUFFER_SIZE];
    char cmd[CMD_BUFFER_SIZE]; 
    char *restOfCmd;
    char** operands = NULL;
    int* machineCodes = NULL;
    SYMBOL_TABLE_ENTRY* pSymbol;

    while (fgets(cmdLine, LINE_BUFFER_SIZE, fin) != NULL)
    {
        lineCnt++;
        
        // Handle the case with a line bigger then supported.
        while (strlen(cmdLine) > LINE_SIZE && fgets(cmdLine, LINE_BUFFER_SIZE, fin) != NULL)
        {
            lineWasTooBig = TRUE;
        }

        if (lineWasTooBig)
        {
            PRINT_ERR(lineCnt, "The line has too many characters. Maximum 80 is supported.");
            lineWasTooBig = FALSE;
            continue;
        }

        restOfCmd = &cmdLine[0];
        if (check_empty(restOfCmd) || is_comment_line(restOfCmd))
        {
            continue;
        }

        free_variables(1, machineCodes);
        machineCodes = NULL;

        status = get_label(restOfCmd, &restOfCmd, label, &found_label, lineCnt);
        if (!status)
        {
            found_error = TRUE;
            continue;
        }
        status = get_cmd(restOfCmd, &restOfCmd, cmd, lineCnt);
        if (!status)
        {
            found_error = TRUE;
            continue;
        }
        if (is_data_cmd(cmd))  
        {
            if (found_label)
            {
                status = add_label_to_symbol_table(pSymbolTable, label, DATA, dc, lineCnt);
                if (!status)
                {
                    found_error = TRUE;
                    continue;
                }
            }
            operandsCnt = prepare_operands_for_cmd(cmd, restOfCmd, &operands, lineCnt);
            if (operandsCnt < 0)
            {
                found_error = TRUE;
                continue;
            }
            machineCodesLength = build_machine_code(cmd, operands, operandsCnt, &machineCodes, lineCnt);
            free_array_of_strings(operands, operandsCnt);
            if (machineCodesLength < 0)
            {
                found_error = TRUE;
                continue;
            }
            add_entries_to_data_table(pDataTable, machineCodes, machineCodesLength);
            dc += machineCodesLength;
            
        }
        else if (is_extern(cmd))
        {
            if (found_label)
            {
                PRINT_WARN(lineCnt, "The extern directive label is ignored.");
            }
            operandsCnt = prepare_operands_for_cmd(cmd, restOfCmd, &operands, lineCnt);
            if (operandsCnt < 0)
            {
                found_error = TRUE;
                continue;
            }
            pSymbol = get_symbol_entry(pSymbolTable, operands[0]);
            if (pSymbol)    // if the symbol is already defined
            {
                PRINT_ERR(lineCnt, "The symbol is already defined in the current file.");
                found_error = TRUE;
                continue;
            }
            add_entry_to_symbol_table(pSymbolTable, operands[0], EXTERNAL, 0);
            add_label_to_externals_table(pExternalsTable, operands[0]);
            free_array_of_strings(operands, 1);
        }
        else if (!is_entry(cmd))    // for all commands other then entry
        {   
            if (found_label)
            {
                status = add_label_to_symbol_table(pSymbolTable, label, CODE, ic, lineCnt);
                if (!status)
                {
                    found_error = TRUE;
                    continue;
                }
            }
            operandsCnt = prepare_operands_for_cmd(cmd, restOfCmd, &operands, lineCnt);
            if (operandsCnt < 0)
            {
                found_error = TRUE;
                continue;
            }
            machineCodesLength = build_machine_code(cmd, operands, operandsCnt, &machineCodes, lineCnt);
            free_array_of_strings(operands, operandsCnt);
            if (machineCodesLength < 0)
            {
                found_error = TRUE;
                continue;
            }
            add_entry_to_code_table(pCmdTable, machineCodes, machineCodesLength, ic);
            ic += machineCodesLength;
        }
        else  // entry
        {
            if (found_label)
            {
                PRINT_WARN(lineCnt, "The entry directive label is ignored.");
            }
        }
    }

    free_variables(1, machineCodes);
    *icf = ic;
    *dcf = dc;
    complete_symbols_addresses_for_data(pSymbolTable, ic);
    // TODO below is for debug
    //printf("ICF - %d, DCF - %d\n", ic, dc);
    //print_symbol_table(pSymbolTable, stdout);
    //print_data_table(pDataTable, stdout);
    //print_cmd_table(pCmdTable, stdout);
    return !found_error;
}

/**
*
*/
BOOLEAN second_pass(FILE *fin, SYMBOL_TABLE* pSymbolTable, CMD_TABLE *pCmdTable, DATA_TABLE *pDataTable, EXTERNALS_TABLE* pExternalsTable)
{
    BOOLEAN found_error = FALSE, status, foundLabel;
    char cmdLine[LINE_BUFFER_SIZE];
    char label[LABEL_BUFFER_SIZE]; 
    char cmd[CMD_BUFFER_SIZE];
    int lineCnt = 0, cmdCnt = 0;
    char *restOfCmd;

    if (fseek(fin, 0, SEEK_SET))
    {
        PRINT_ERR(0, "Couldn't parse the input file.")
        return FALSE;
    }

    while (fgets(cmdLine, LINE_SIZE+1, fin) != NULL)
    {
        lineCnt++;
        restOfCmd = &cmdLine[0];
        if (check_empty(restOfCmd) || is_comment_line(restOfCmd))
        {
            continue;
        }
        get_label(restOfCmd, &restOfCmd, label, &foundLabel, lineCnt);    // skip label
        status = get_cmd(restOfCmd, &restOfCmd, cmd, lineCnt);
        if (!status)
        {
            found_error = TRUE;
            continue;
        }
        if (is_entry(cmd))
        {
            if (!handle_entry_cmd(cmd, restOfCmd, lineCnt, pSymbolTable))
            {
                found_error = TRUE;
            }
        }
        else if (!(is_extern(cmd) || is_data_cmd(cmd))) // code cmd
        {
            if (!complete_cmd_in_cmd_table(pCmdTable, pSymbolTable, pExternalsTable, cmd, restOfCmd, cmdCnt++, lineCnt))
            {
                found_error = TRUE;
            }
        }
    }

    return !found_error;
}

void select_entries(SYMBOL_TABLE_ENTRY *pSymbol, LinkedList *pOutList)
{
    if (pSymbol->type & ENTRY)
    {
        add_entry_to_list(pOutList, pSymbol, sizeof(SYMBOL_TABLE_ENTRY));
    }
}

void printEntryFromSymbolTable(Node* pNode, void* additionalInfo)
{
    FILE *fout = (FILE*)additionalInfo;
    SYMBOL_TABLE_ENTRY *pSymbol = (SYMBOL_TABLE_ENTRY*)(pNode->data);
    fprintf(fout, "%s %07d\n", pSymbol->label, pSymbol->addr);
}

/**
* Generates the following:
* 1. fileName.ob with code and data
* 2. fileName.ent with a list of entries and thier addresses. In case of no entries in the code, no file is generated.
* 3. fileName.ext with a list of externals and the addresses where they are used. In case of no externals in the code, no file is generated.
*/
BOOLEAN generate_output(char* fileName, SYMBOL_TABLE* pSymbolTable, CMD_TABLE* pCmdTable, DATA_TABLE* pDataTable, 
    EXTERNALS_TABLE* pExternalsTable, int icf, int dcf)
{
    char *obFileName = NULL, *entriesFileName = NULL, *externalFileName = NULL;
    FILE *obFile = NULL, *entriesFile = NULL, *externalsFile = NULL;
    LinkedList entries = { NULL };
    BOOLEAN status = TRUE;

    obFileName = concat(fileName, ".ob");
    RETURN_ON_MEMORY_FAILURE(obFileName, FALSE);
    obFile = fopen(obFileName, "w");
    GOTO_LABEL_ON_NULL(obFile, prepare_to_return, status, FALSE, 0, "failed to open a file for .obj\n");
    fprintf(obFile, "%7d %d\n", icf - COMMAND_SECTION_START, dcf);
    print_cmd_table(pCmdTable, obFile, COMMAND_SECTION_START);
    print_data_table(pDataTable, obFile, icf);
    select_from_symbol_table(pSymbolTable, select_entries, &entries);
    if (entries.head)
    {
        entriesFileName = concat(fileName, ".ent");
        GOTO_LABEL_ON_NULL(entriesFileName, prepare_to_return, status, FALSE, 0, "Failed to allocate memory.\n");
        entriesFile = fopen(entriesFileName, "w");
        GOTO_LABEL_ON_NULL(entriesFile, prepare_to_return, status, FALSE, 0, "Failed to open file for .ent.\n");
        apply_to_linked_list(&entries, printEntryFromSymbolTable, entriesFile);
    }
    if (pExternalsTable->Head)
    {
        externalFileName = concat(fileName, ".ext");
        GOTO_LABEL_ON_NULL(externalFileName, prepare_to_return, status, FALSE, 0, "Failed to allocate memory.\n");
        externalsFile = fopen(externalFileName, "w");
        GOTO_LABEL_ON_NULL(externalsFile, prepare_to_return, status, FALSE, 0, "Failed to open file for .ext.\n");
        print_externals_table(pExternalsTable, externalsFile);
    }

prepare_to_return:
    free_variables(3, obFileName, entriesFileName, externalFileName);
    if (obFile)
    {
        fclose(obFile);
    }
    if (entriesFile)
    {
        fclose(entriesFile);
    }
    if (externalsFile)
    {
        fclose(externalsFile);
    }
    
    return status;
}

BOOLEAN compile(FILE* fin, char* fileName)
{
    BOOLEAN status = TRUE;
    int icf;
    int dcf;
    SYMBOL_TABLE *pSymbolTable = init_symbol_table();
    CMD_TABLE *pCmdTable = init_cmd_table();
    DATA_TABLE *pDataTable = init_data_table();
    EXTERNALS_TABLE *pExternalsTable = init_externals_table();

    if (!(pSymbolTable && pCmdTable && pDataTable && pExternalsTable))
    {
        printf("Couldn't allocate tables, exiting...\n");
        status = FALSE;
        goto prepare_to_return;
    }

    status = first_pass(fin, pSymbolTable, pCmdTable, pDataTable, pExternalsTable, &icf, &dcf);
    if (!status)
    {
        goto prepare_to_return;
    }
    status = second_pass(fin, pSymbolTable, pCmdTable, pDataTable, pExternalsTable);
    if (!status)
    {
        goto prepare_to_return;
    }

    status = generate_output(fileName, pSymbolTable, pCmdTable, pDataTable, pExternalsTable, icf, dcf);
    if (!status)
    {
        goto prepare_to_return;
    }

prepare_to_return:
    release_symbol_table(pSymbolTable);
    release_cmd_table(pCmdTable);
    release_data_table(pDataTable);
    release_externals_table(pExternalsTable);
    return status;
    
}





