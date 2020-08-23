#ifndef __TABLES__
#define __TABLES__

#include "Utilities.h"

// Enum to be used to differentiate between different symbols in the symbol table.
typedef enum
{
    CODE = 1,
    DATA = 2,
    EXTERNAL = 4,
    ENTRY = 8
} SYMBOL_TYPE;

typedef struct _SYMBOL_TABLE_ENTRY
{
    char* label;        // label as it came from the command line.
    int addr;           // The symbol address in the code space.
    SYMBOL_TYPE type;   // Type - bit mask to hold the symbol characteristics. 
    struct _SYMBOL_TABLE_ENTRY* next;   // pointer to the next symbol in the linked list.
} SYMBOL_TABLE_ENTRY;

typedef struct 
{
    SYMBOL_TABLE_ENTRY* Head;   // pointer to the first symbol in the linked list.
} SYMBOL_TABLE;

typedef struct _CMD_TABLE_ENTRY
{
    int* MachineCodes;          // array to hold machine codes for a command.
    int MachineCodesLength;     // length of the machine codes array.
    int cmdAddress;             // address of the command in the command space.
    struct _CMD_TABLE_ENTRY* next; // pointer to the next command in the linked list.
} CMD_TABLE_ENTRY;

typedef struct
{
    CMD_TABLE_ENTRY* Head;      // pointer to the first command in the linked list. 
} CMD_TABLE;

typedef struct _DATA_TABLE_ENTRY
{
    int data;                           // data encoded to the machine word.
    struct _DATA_TABLE_ENTRY* next;     // pointer to the next data element.
} DATA_TABLE_ENTRY;

typedef struct 
{
    DATA_TABLE_ENTRY* Head;    // pointer to the first data element in the linked list.          
} DATA_TABLE;

typedef struct _EXTERNAL_TABLE_ENTRY
{
    char* extLabel;                     // external label as it came from command line.
    LinkedList* pExternalCodeAddresses; // linked list of addresses where the external is referenced.
    struct _EXTERNAL_TABLE_ENTRY* next; // pointer to the next element in the external table.
} EXTERNALS_TABLE_ENTRY;

typedef struct
{
    EXTERNALS_TABLE_ENTRY* Head;        // pointer to the first external element in the linked list.
} EXTERNALS_TABLE;

/**
* Creates a new entry in the symbol table with the provided label and add it to the table. 
* On success TRUE, otherwise FALSE.
*/
BOOLEAN add_entry_to_symbol_table(SYMBOL_TABLE* pSymbolTable, char* symbol, SYMBOL_TYPE symType, int addr);

typedef void(*SELECT_PREDICATE)(SYMBOL_TABLE_ENTRY*, LinkedList *pOutList);

/**
* This function selects entries from the symbol table based on given predicate, the selected items
* are placed into pOutList.
*/
void select_from_symbol_table(SYMBOL_TABLE* pSymbolTable, SELECT_PREDICATE predicate, LinkedList *pOutList);

/**
* Gets entry for the given symbol from the symbol table.
* In case symbol not found in the table return NULL.
*/
SYMBOL_TABLE_ENTRY* get_symbol_entry(SYMBOL_TABLE* pSymbolTable, char* symbol);

typedef void(*ACTION_TO_APPLY_TO_SYMBOL_TABLE)(SYMBOL_TABLE_ENTRY*, void*);

/**
* Applies given function to each element. Together with the element, it provided the additionalInfo to funcToApply
* as it passed by the caller.
*/
void apply_to_symbol_table_entries(SYMBOL_TABLE* pSymbolTable, ACTION_TO_APPLY_TO_SYMBOL_TABLE funcToApply, void* additionalInfo);

/**
* Creates a new entry in the code table with the provided machine codes and adds it to it.
* On success TRUE, otherwise FALSE
*/
BOOLEAN add_entry_to_code_table(CMD_TABLE* pCmdTable, int* machineCodes, int length, int ic);

/**
* Creates a new entry in the data table with the provided machine codes and adds it to it.
* On success TRUE, otherwise FALSE
*/
BOOLEAN add_entries_to_data_table(DATA_TABLE* pDataTable, int* machineCodes, int length);

/**
* Creates a new entry in the external table with the provided label
* On success TRUE, otherwise FALSE
*/
BOOLEAN add_label_to_externals_table(EXTERNALS_TABLE* pExternalsTable, char* label);

/**
* Adds a new code address for the given label in the externals table.
* On success TRUE, otherwise FALSE
*/
BOOLEAN add_call_addr_to_external(EXTERNALS_TABLE* pExternalsTable, char* externalLabel, int ic);

/**
* Creates and initializes symbol table. In case of failure will return NULL.
*/
SYMBOL_TABLE* init_symbol_table();

/**
* Creates and initializes command table. In case of failure will return NULL.
*/
CMD_TABLE* init_cmd_table();

/**
* Creates and initializes data table. In case of failure will return NULL.
*/
DATA_TABLE* init_data_table();

/**
* Creates and initializes data table. In case of failure will return NULL.
*/
EXTERNALS_TABLE* init_externals_table();

/**
* Releases symbol table
*/
void release_symbol_table(SYMBOL_TABLE* pSymbolTable);
/**
* Releases cmd table
*/
void release_cmd_table(CMD_TABLE* pCmdTable);
/**
* Releases data table
*/
void release_data_table(DATA_TABLE* pDataTable);

/**
* Releases externals table
*/
void release_externals_table(EXTERNALS_TABLE* pExternalsTable);

/**
* Gets entry which in position of the given entryIndex.
*/
CMD_TABLE_ENTRY* get_cmd_table_entry(CMD_TABLE* pCmdTable, int entryIndex);

/*
* Prints data table to fout.
*/
void print_data_table(DATA_TABLE* pSymbolTable, FILE* fout, int startAddr);

/*
* Prints cmd table to fout.
*/
void print_cmd_table(CMD_TABLE* pCmdTable, FILE* fout, int startAddr);

/*
* Prints externals table to fout.
*/
void print_externals_table(EXTERNALS_TABLE* pExternalsTable, FILE* fout);

#endif