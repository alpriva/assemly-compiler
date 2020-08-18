#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Tables.h"

BOOLEAN add_entry_to_symbol_table(SYMBOL_TABLE* pSymbolTable, char* symbol, SYMBOL_TYPE symType, int addr)
{
    SYMBOL_TABLE_ENTRY *pCur;
    SYMBOL_TABLE_ENTRY *new_entry = (SYMBOL_TABLE_ENTRY*)malloc(sizeof(SYMBOL_TABLE_ENTRY));
    if (!new_entry)
    {
        return FALSE;
    }
    new_entry->label = (char*)malloc(sizeof(strlen(symbol)+1));   
    if (!new_entry->label)
    {
        free(new_entry);
        return FALSE;
    }
    strcpy(new_entry->label, symbol);
    new_entry->addr = addr;
    new_entry->type = symType; 
    new_entry->next = NULL;
    pCur = pSymbolTable->Head;
    if (pCur)
    {
        while (pCur->next)
        {
            pCur = pCur->next;
        }
        pCur->next = new_entry;
    }
    else 
    {
        pSymbolTable->Head = new_entry;
    }

    return TRUE;
}

void select_from_symbol_table(SYMBOL_TABLE* pSymbolTable, SELECT_PREDICATE predicate, LinkedList* pOutList)
{
    SYMBOL_TABLE_ENTRY *pCur = pSymbolTable->Head;
    while (pCur)
    {
        predicate(pCur, pOutList);
        pCur = pCur->next;
    }
}

BOOLEAN add_entry_to_code_table(CMD_TABLE* pCmdTable, int* machineCodes, int length, int ic)
{
    CMD_TABLE_ENTRY *pCur;
    CMD_TABLE_ENTRY *new_entry = (CMD_TABLE_ENTRY*)malloc(sizeof(CMD_TABLE_ENTRY));
    if (!new_entry)
    {
        return FALSE;
    }
    new_entry->MachineCodes = (int*)malloc(sizeof(int) * length);
    if (!new_entry->MachineCodes)
    {
        free(new_entry);
        return FALSE;
    }
    memcpy(new_entry->MachineCodes, machineCodes, sizeof(int) * length);
    new_entry->MachineCodesLength = length;
    new_entry->cmdAddress = ic;
    new_entry->next = NULL;
    pCur = pCmdTable->Head;
    if (pCur)
    {
        while (pCur->next)
        {
            pCur = pCur->next;
        }
        pCur->next = new_entry;
    }
    else
    {
        pCmdTable->Head = new_entry;
    }

    return TRUE;
}

BOOLEAN add_entry_to_data_table(DATA_TABLE* pDataTable, int machineCode)
{
    DATA_TABLE_ENTRY *pCur;
    DATA_TABLE_ENTRY *new_entry = (DATA_TABLE_ENTRY*)malloc(sizeof(DATA_TABLE_ENTRY));
    if (!new_entry)
    {
        return FALSE;
    }
    new_entry->data = machineCode;
    new_entry->next = NULL;
    pCur = pDataTable->Head;
    if (pCur)
    {
        while (pCur->next)
        {
            pCur = pCur->next;
        }
        pCur->next = new_entry;
    }
    else
    {
        pDataTable->Head = new_entry;
    }

    return TRUE;
}

BOOLEAN add_entries_to_data_table(DATA_TABLE* pDataTable, int* machineCodes, int length)
{
    int i;
    for (i = 0; i < length; i++)
    {
        if (!add_entry_to_data_table(pDataTable, machineCodes[i]))
        {
            return FALSE;
        }
    }
    return TRUE;
}

BOOLEAN add_label_to_externals_table(EXTERNALS_TABLE* pExternalsTable, char* label)
{
    EXTERNALS_TABLE_ENTRY *pCur = pExternalsTable->Head;
    EXTERNALS_TABLE_ENTRY *pNewElem = (EXTERNALS_TABLE_ENTRY*)malloc(sizeof(EXTERNALS_TABLE_ENTRY));
    if (!pNewElem)
    {
        return FALSE;   // TODO switch to MACRO - NOT_ENOUGH_MEMORY_ERROR
    }
    pNewElem->next = NULL;
    pNewElem->extLabel = (char*)malloc(strlen(label) + 1);
    if (!pNewElem->extLabel)
    {
        return FALSE;   // TODO switch to MACRO - NOT_ENOUGH_MEMORY_ERROR
    }
    strcpy(pNewElem->extLabel, label);
    pNewElem->pExternalCodeAddresses = init_linked_list();
    if (!pCur)  // table is empty
    {
        pExternalsTable->Head = pNewElem;
    }
    else
    {
        while (pCur->next)
        {
            pCur = pCur->next;
        }
        pCur->next = pNewElem;
    }
    return TRUE;
}

BOOLEAN add_call_addr_to_external(EXTERNALS_TABLE* pExternalsTable, char* externalLabel, int ic)
{
    EXTERNALS_TABLE_ENTRY *pCur = pExternalsTable->Head;
    while (pCur)
    {
        if (strcmp(pCur->extLabel, externalLabel) == 0)
        {
            add_entry_to_list(pCur->pExternalCodeAddresses, &ic, 4);    // first 4 byte is enough, as the address cannot be more than 21 bit length.
            return TRUE;
        }
        pCur = pCur->next;
    }
    return FALSE;
}

SYMBOL_TABLE* init_symbol_table()
{
    SYMBOL_TABLE *table = (SYMBOL_TABLE*)malloc(sizeof(SYMBOL_TABLE));
    if (table)
    {
        table->Head = NULL;
    }
    return table;
}

CMD_TABLE* init_cmd_table()
{
    CMD_TABLE *table = (CMD_TABLE*)malloc(sizeof(CMD_TABLE));
    if (table)
    {
        table->Head = NULL;
    }
    return table;
}

DATA_TABLE* init_data_table()
{
    DATA_TABLE *table = (DATA_TABLE*)malloc(sizeof(DATA_TABLE));
    if (table)
    {
        table->Head = NULL;
    }
    return table;
}

EXTERNALS_TABLE* init_externals_table()
{
    EXTERNALS_TABLE *table = (EXTERNALS_TABLE*)malloc(sizeof(EXTERNALS_TABLE));
    if (table)
    {
        table->Head = NULL;
    }
    return table;
}

void release_symbol_table(SYMBOL_TABLE *pSymbolTable) 
{
    SYMBOL_TABLE_ENTRY *pCurrentEntry = pSymbolTable->Head;
    SYMBOL_TABLE_ENTRY *pNextEntry;
    while (pCurrentEntry)
    {
        pNextEntry = pCurrentEntry->next;
        free(pCurrentEntry);
        pCurrentEntry = pNextEntry;
    }
    free(pSymbolTable);
}

void release_cmd_table(CMD_TABLE *pCmdTable)
{
    CMD_TABLE_ENTRY *pCurrentEntry = pCmdTable->Head;
    CMD_TABLE_ENTRY *pNextEntry;
    while (pCurrentEntry)
    {
        pNextEntry = pCurrentEntry->next;
        free(pCurrentEntry);
        pCurrentEntry = pNextEntry;
    }
    free(pCmdTable);
}

void release_data_table(DATA_TABLE *pDataTable)
{
    DATA_TABLE_ENTRY *pCurrentEntry = pDataTable->Head;
    DATA_TABLE_ENTRY *pNextEntry;
    while (pCurrentEntry)
    {
        pNextEntry = pCurrentEntry->next;
        free(pCurrentEntry);
        pCurrentEntry = pNextEntry;
    }
    free(pDataTable);
}

void release_externals_table(EXTERNALS_TABLE* pExternalsTable)
{
    EXTERNALS_TABLE_ENTRY *pCurrentEntry = pExternalsTable->Head;
    EXTERNALS_TABLE_ENTRY *pNextEntry;
    while (pCurrentEntry)
    {
        pNextEntry = pCurrentEntry->next;
        free(pCurrentEntry->extLabel);
        free_linked_list(pCurrentEntry->pExternalCodeAddresses);
        free(pCurrentEntry);
        pCurrentEntry = pNextEntry;
    }
    free(pExternalsTable);
}

CMD_TABLE_ENTRY* get_cmd_table_entry(CMD_TABLE* pCmdTable, int entryIndex)
{
    CMD_TABLE_ENTRY *pCur = pCmdTable->Head;
    int curCnt = 0;

    while (pCur)
    {
        if (curCnt == entryIndex)
        {
            return pCur;
        }
        curCnt++;
        pCur = pCur->next;
    }
    return NULL;
}

SYMBOL_TABLE_ENTRY* get_symbol_entry(SYMBOL_TABLE* pSymbolTable, char* symbol)
{
    SYMBOL_TABLE_ENTRY *pCur = pSymbolTable->Head;
    while (pCur)
    {
        if (strcmp(pCur->label, symbol) == 0)
        {
            return pCur;
        }
        pCur = pCur->next;
    }
    return NULL;
}

void apply_to_symbol_table_entries(SYMBOL_TABLE* pSymbolTable, ACTION_TO_APPLY_TO_SYMBOL_TABLE funcToApply, void* additionalInfo)
{
    SYMBOL_TABLE_ENTRY *pCur = pSymbolTable->Head;
    while (pCur)
    {
        funcToApply(pCur, additionalInfo);
        pCur = pCur->next;
    }
}

void print_symbol_table(SYMBOL_TABLE* pSymbolTable, FILE* fout)
{
    SYMBOL_TABLE_ENTRY *pCur = pSymbolTable->Head;
    fprintf(fout, "Symbol table content:\n");
    while (pCur)
    {
        fprintf(fout, "%32s |\t%3d\t| %x\n", pCur->label, pCur->addr, pCur->type);
        pCur = pCur->next;
    }
}

void print_data_table(DATA_TABLE* pSymbolTable, FILE* fout, int startAddr)
{
    int lineCnt = startAddr;
    DATA_TABLE_ENTRY *pCur = pSymbolTable->Head;
    while (pCur)
    {
        fprintf(fout, "%07d %06x\n", lineCnt++, pCur->data);
        pCur = pCur->next;
    }
}

void print_cmd_table(CMD_TABLE* pCmdTable, FILE* fout, int startAddr)
{
    int lineCnt = startAddr, i;
    CMD_TABLE_ENTRY *pCur = pCmdTable->Head;
    while (pCur)
    {
        for (i = 0; i < pCur->MachineCodesLength; i++)
        {
            fprintf(fout, "%07d %06x\n", lineCnt++, pCur->MachineCodes[i]);
        }
        
        pCur = pCur->next;
    }
}

void print_externals_table(EXTERNALS_TABLE* pExternalsTable, FILE* fout)
{
    EXTERNALS_TABLE_ENTRY *pCurExternal = pExternalsTable->Head;
    Node *pCurAddress;
    while (pCurExternal)
    {
        pCurAddress = pCurExternal->pExternalCodeAddresses->head;
        while (pCurAddress)
        {
            fprintf(fout, "%s %07d\n", pCurExternal->extLabel, *(int*)(pCurAddress->data));
            pCurAddress = pCurAddress->next;
        }
        pCurExternal = pCurExternal->next;
    }
}

