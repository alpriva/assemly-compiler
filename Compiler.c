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
        PRINT_ERR(lineCnt, "No such symbol - %s.", operands[0]);
        status = FALSE;
    }
    else if (pSymbolTableEntry->type & EXTERNAL)
    {
        PRINT_ERR(lineCnt, "The external label - %s in the current file cannot be entry.", operands[0]);
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
* Each command, which references a symbol is considered to be incomplete prior to running this method.
* After this method runs, all the commands in the command table should be valid.
* In case of failure return FALSE, else TRUE.
*/
BOOLEAN complete_cmd_in_cmd_table(CMD_TABLE* pCmdTable, SYMBOL_TABLE* pSymbolTable, EXTERNALS_TABLE* pExternalTable, char* cmd, char* commandLine, int cmdCnt, int lineCnt)
{
    // pointer to the command table entry
    CMD_TABLE_ENTRY *pCmdTableEntry;
    // pointer to the symbol table entry
    SYMBOL_TABLE_ENTRY *pSymbolTableEntry;
    // operandsCnt - holds the number of operands for the command.
    int i, operandsCnt;
    // temp pointer to hold the label.
    char *label = NULL;
    // pointer to a string array to hold command's operands.
    char **operands = NULL;
    // intermediate variable to hold different commands return statuses.
    BOOLEAN status = TRUE;

    operandsCnt = prepare_operands_for_cmd(cmd, commandLine, &operands, lineCnt);
    if (operandsCnt < 0)
    {
        PRINT_ERR(lineCnt, "Couldn't parse operands for command - %s.", cmd);
        return FALSE;
    }

    pCmdTableEntry = get_cmd_table_entry(pCmdTable, cmdCnt);
    if (!pCmdTableEntry)
    {
        PRINT_ERR(lineCnt, "The command is unexpected");
        return FALSE;
    }

    for (i = 1; i < pCmdTableEntry->MachineCodesLength; i++)    // starting from the second word, as the first is always a command
    {
        if (!is_valid_machine_code(*(pCmdTableEntry->MachineCodes + i)))    // the machine code is not valid only in case of a label
        {
            // get the i-1 label in the operands array. it can have the '&' sign as the first char.
            label = get_label_from_operands(operands, operandsCnt, i - 1);
            if (!label)
            {
                PRINT_ERR(lineCnt, "Label not found in operands of the command: %s.", cmd);
                status = FALSE;
                goto prepare_for_exit;
            }

            if (is_relative(label))
            {
                pSymbolTableEntry = get_symbol_entry(pSymbolTable, label + 1);  // skip the '&' sign.
            }
            else
            {
                pSymbolTableEntry = get_symbol_entry(pSymbolTable, label);
            }
            
            if (!pSymbolTableEntry)
            {
                PRINT_ERR(lineCnt, "No such symbol - %s.", label);
                status = FALSE;
                goto prepare_for_exit;
            }

            if (pSymbolTableEntry->type & EXTERNAL && is_relative(label))   // external reference is not allowed with relative address resolution.
            {
                PRINT_ERR(lineCnt, "Invalid operand - %s.", label);
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
                // Add external variable reference address to the externals table.
                if (!add_call_addr_to_external(pExternalTable, pSymbolTableEntry->label, pCmdTableEntry->cmdAddress + i))
                {
                    PRINT_ERR(lineCnt, "Symbol %s is undefined in the externals list.", label);
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

/**
* Updates the symbolEntry with end of instruction pointer
*/
void add_end_of_instructions_to_data_symbols(SYMBOL_TABLE_ENTRY* symbolEntry, void* additionalInfo)
{
    int icf = *((int*)additionalInfo);
    if (symbolEntry->type & DATA)
    {
        symbolEntry->addr += icf;
    }
}

/**
* Adds icf to all the data symbols in the symbol table.
*/
void complete_symbols_addresses_for_data(SYMBOL_TABLE* pSymbolTable, int icf)
{
    apply_to_symbol_table_entries(pSymbolTable, add_end_of_instructions_to_data_symbols, &icf);
}

/**
* Adds label to the symbol table. In case it exists it returns FALSE.
*/
BOOLEAN add_label_to_symbol_table(SYMBOL_TABLE* pSymbolTable, char* label, SYMBOL_TYPE labelType, int addr, int lineCnt)
{
    SYMBOL_TABLE_ENTRY *pSymbol;
    
    pSymbol = get_symbol_entry(pSymbolTable, label);
    if(pSymbol)
    {
        PRINT_ERR(lineCnt, "The label - %s is already defined.", label);
        return FALSE;
    }
    return add_entry_to_symbol_table(pSymbolTable, label, labelType, addr);
}

/**
* Performs the first pass of the compiler algorithm.
* In this pass each label/external is added to the symbol table with it's address.
* Each command/directive is compiled. If labels are used in the command, empty words are used. 
* These empty words will be filled on the next pass.
* Returns TRUE if successefull, otherwise FALSE.
*/
BOOLEAN first_pass(FILE* fin, SYMBOL_TABLE* pSymbolTable, CMD_TABLE* pCmdTable, DATA_TABLE* pDataTable, EXTERNALS_TABLE* pExternalsTable, int *icf, int *dcf)
{
    // Used to store the command line as it read from the source file.
    char cmdLine[LINE_BUFFER_SIZE];
    // ic - instructions code address, dc - data code addresses.
    int dc = DATA_SECTION_START, ic = COMMAND_SECTION_START;
    // LineCnt - holds the current line count. machineCodesLength - holds the length of a command in words. 
    // operandsCnt - holds the number of operands in a command.
    int lineCnt = 0, machineCodesLength, operandsCnt;
    // found_error - indicates if error was found during the first pass. status - variable to hold operation statuses. 
    // found_label - indicates if label was found in the current line. lineWasTooBig - indicate if the current line was too big.
    BOOLEAN found_error = FALSE, status, found_label, lineWasTooBig = FALSE;
    // buffer to hold the label
    char label[LABEL_BUFFER_SIZE];
    // buffer to hold the command or directive
    char cmd[CMD_BUFFER_SIZE]; 
    // pointer to the rest of command which is yet processed.
    char *restOfCmd;
    // string array of a command operands.
    char** operands = NULL;
    // array of machine code words for a command.
    int* machineCodes = NULL;
    // pointer to a symbol table.
    SYMBOL_TABLE_ENTRY* pSymbol;

    while (fgets(cmdLine, LINE_BUFFER_SIZE, fin) != NULL)
    {
        lineCnt++;
        
        // Handle the case with a line bigger then supported.
        while (strlen(cmdLine) > LINE_SIZE)
        {
            lineWasTooBig = TRUE;   // Indicate that line was too big, to be handled below.
            if (fgets(cmdLine, LINE_BUFFER_SIZE, fin) == NULL)  // if end of file - break.
            {
                break;
            }
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

        // start by making sure intermediate array is freed and null.
        free_variables(1, machineCodes);    
        machineCodes = NULL;

        status = get_label(restOfCmd, &restOfCmd, label, &found_label, lineCnt);
        if (!status)
        {
            // the label had wrong syntax
            found_error = TRUE;
            continue;
        }

        status = get_cmd(restOfCmd, &restOfCmd, cmd, lineCnt);
        if (!status)
        {
            // command is invalid or not found.
            found_error = TRUE;
            continue;
        }

        if (is_data_cmd(cmd))  // in case of data directive
        {
            if (found_label)    
            {
                status = add_label_to_symbol_table(pSymbolTable, label, DATA, dc, lineCnt);
                if (!status)
                {
                    // Couldn't add label to the table.
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

            status = add_entries_to_data_table(pDataTable, machineCodes, machineCodesLength);
            if (!status)
            {
                PRINT_NO_MEMORY_ERR();
                found_error = TRUE;
                continue;
            }
            dc += machineCodesLength;
        }
        else if (is_extern(cmd))
        {
            if (found_label)
            {
                PRINT_WARN(lineCnt, "The extern directive label - %s, is ignored.", label);
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
                PRINT_ERR(lineCnt, "The symbol - %s is already defined in the current file.", operands[0]);
                free_array_of_strings(operands, 1);
                found_error = TRUE;
                continue;
            }

            status = add_entry_to_symbol_table(pSymbolTable, operands[0], EXTERNAL, 0);
            if (!status)
            {
                PRINT_NO_MEMORY_ERR();
                free_array_of_strings(operands, 1);
                found_error = TRUE;
                continue;
            }

            status = add_label_to_externals_table(pExternalsTable, operands[0]);
            if (!status)
            {
                PRINT_NO_MEMORY_ERR();
                found_error = TRUE;
            }
            free_array_of_strings(operands, 1);
        }
        else if (!is_entry(cmd))    // for all the commands other then entry
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

            status = add_entry_to_code_table(pCmdTable, machineCodes, machineCodesLength, ic);
            if (!status)
            {
                PRINT_NO_MEMORY_ERR();
                found_error = TRUE;
            }
            ic += machineCodesLength;
        }
        else  // entry
        {
            if (found_label)
            {
                PRINT_WARN(lineCnt, "The entry directive label - %s is ignored.", label);
            }
        }
    }

    free_variables(1, machineCodes);
    *icf = ic;
    *dcf = dc;
    complete_symbols_addresses_for_data(pSymbolTable, ic);

    return !found_error;
}

/**
* Performs the second pass of the algorithm.
* It performs additional path on the source file.
* It assigns entry flags to the symbols stored in symbol table, in case of entry command.
* It completes the commands translation with label addresses.
*/
BOOLEAN second_pass(FILE *fin, SYMBOL_TABLE* pSymbolTable, CMD_TABLE *pCmdTable, DATA_TABLE *pDataTable, EXTERNALS_TABLE* pExternalsTable)
{
    // found_error - indicates if error was found during the second pass. 
    // status - intermediate variable to hold different operations return status.
    // foundLabel - indicates if label was found in the current line.
    BOOLEAN found_error = FALSE, status, foundLabel;
    // buffer to hold the command line.
    char cmdLine[LINE_BUFFER_SIZE];
    // buffer to hold the label.
    char label[LABEL_BUFFER_SIZE]; 
    // buffer to hold the commands.
    char cmd[CMD_BUFFER_SIZE];
    // lineCnt - current line number. cmdCnt - current command index.
    int lineCnt = 0, cmdCnt = 0;
    // pointer to the rest of command.
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
        if (check_empty(restOfCmd) || is_comment_line(restOfCmd))   // skip commend or empty lines.
        {
            continue;
        }
        
        get_label(restOfCmd, &restOfCmd, label, &foundLabel, lineCnt);    // skip label, as it is was handled during the first pass.
        
        status = get_cmd(restOfCmd, &restOfCmd, cmd, lineCnt);
        if (!status)
        {
            found_error = TRUE;
            continue;
        }

        if (is_entry(cmd))
        {
            if (!handle_entry_cmd(cmd, restOfCmd, lineCnt, pSymbolTable))   // handles the entry command.
            {
                found_error = TRUE;
            }
        }
        else if (!(is_extern(cmd) || is_data_cmd(cmd))) // code cmd
        {
            // complete the command if needed using the populated symbol table.
            if (!complete_cmd_in_cmd_table(pCmdTable, pSymbolTable, pExternalsTable, cmd, restOfCmd, cmdCnt++, lineCnt))
            {
                found_error = TRUE;
            }
        }
    }

    return !found_error;
}

/**
* Selects symbols if they are entries and adds them to the pOutList.
*/
void select_entries(SYMBOL_TABLE_ENTRY *pSymbol, LinkedList *pOutList)
{
    if (pSymbol->type & ENTRY)  // if a command has an entry flag.
    {
        add_entry_to_list(pOutList, pSymbol, sizeof(SYMBOL_TABLE_ENTRY));
    }
}

/**
* Prints entry from symbol table to the file handle stored in additionalInfo.
*/
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
    // obFileName - name of the object file to generate. entriesFileName - name of the entries file name to generate.
    // externalFileName - name of the entries file name to generate.
    char *obFileName = NULL, *entriesFileName = NULL, *externalFileName = NULL;
    // obFile - file handle of the object file to generate. entriesFile - file handle of the entries file name to generate.
    // externalsFile - name of the entries file name to generate.
    FILE *obFile = NULL, *entriesFile = NULL, *externalsFile = NULL;
    // list to hold the symbols from the symbol table with entry flag.
    LinkedList *pEntries = init_linked_list();
    // intermediate variable to hold the statuses from different commands.
    BOOLEAN status = TRUE;

    RETURN_ON_MEMORY_FAILURE(pEntries, FALSE);

    obFileName = concat(fileName, ".ob");
    RETURN_ON_MEMORY_FAILURE(obFileName, FALSE);

    obFile = fopen(obFileName, "w");
    GOTO_LABEL_ON_NULL(obFile, prepare_to_return, status, FALSE, 0, "failed to open a file for .obj\n");

    fprintf(obFile, "%7d %d\n", icf - COMMAND_SECTION_START, dcf);

    print_cmd_table(pCmdTable, obFile, COMMAND_SECTION_START);

    print_data_table(pDataTable, obFile, icf);

    select_from_symbol_table(pSymbolTable, select_entries, pEntries);
    if (pEntries->head)
    {
        entriesFileName = concat(fileName, ".ent");
        GOTO_LABEL_ON_NULL(entriesFileName, prepare_to_return, status, FALSE, 0, "Failed to allocate memory.\n");

        entriesFile = fopen(entriesFileName, "w");
        GOTO_LABEL_ON_NULL(entriesFile, prepare_to_return, status, FALSE, 0, "Failed to open file for .ent.\n");

        apply_to_linked_list(pEntries, printEntryFromSymbolTable, entriesFile);
    }
    free_linked_list(pEntries);
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
    // intermediate variable to hold the statuses from different commands.
    BOOLEAN status = TRUE;
    // number of commands in the source file.
    int icf;
    // number of data instructions in the source file.
    int dcf;
    // symbol table
    SYMBOL_TABLE *pSymbolTable = init_symbol_table();
    // command table
    CMD_TABLE *pCmdTable = init_cmd_table();
    // data table
    DATA_TABLE *pDataTable = init_data_table();
    // externals table
    EXTERNALS_TABLE *pExternalsTable = init_externals_table();

    if (!(pSymbolTable && pCmdTable && pDataTable && pExternalsTable))
    {
        PRINT_NO_MEMORY_ERR();
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





