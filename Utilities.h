#ifndef __UTILITIES__
#define __UTILITIES__

#define PRINT_NO_MEMORY_ERR() printf("Failed to allocate memory, exiting...\n"); 

#define RETURN_ON_MEMORY_FAILURE(pointerToCheck, valToReturn) \
            if (!(pointerToCheck)) \
            { \
                printf("Failed to allocate memory, exiting..."); \
                return (valToReturn); \
            }

#define PRINT_ERR(lineCnt, message) printf("Error: In line %d. %s\n", lineCnt, message);
            
#define GOTO_LABEL_ON_NULL(pointerToCheck, label, statusVar, statusToSet, lineCnt, message) \
            if (!pointerToCheck) \
            { \
                statusVar = statusToSet; \
                PRINT_ERR(lineCnt, message) \
                goto label; \
            }

#define GOTO_LABEL_ON_STATUS_ERR(statusToCheck, label, statusVar, statusToSet, lineCnt, message) \
            GOTO_LABEL_ON_NULL(statusToCheck, label, statusVar, statusToSet, lineCnt, message)

#define RETURN_STATUS_ON_ERR(valueToReturn, lineCnt, message) \
            PRINT_ERR(lineCnt, message) \
            return valueToReturn;

typedef enum
{
    FALSE,
    TRUE
} BOOLEAN;

char* concat(const char *s1, const char *s2);

void free_variables(int count, ...);

typedef struct _Node
{
    void* data;
    struct _Node* next;
} Node;

typedef struct
{
    Node *head;
} LinkedList;

/**
* Allocates new ListOfStrings and assigns NULL to head.
*/
LinkedList* init_linked_list();

/**
* Creates and returns an empty node, with data and next equal NULL.
*/
Node *create_empty_node();

/**
* Adds entry to the list
*/
BOOLEAN add_entry_to_list(LinkedList *pList, void *data, size_t dataLengthInByte);

typedef void(*FUNCTION_TO_APPLY_ON_LINKED_LIST_ENTRIES)(Node* pNode, void* additionalInfo);

/**
* Apply function to each entry of linked list
*/
void apply_to_linked_list(LinkedList *pList, FUNCTION_TO_APPLY_ON_LINKED_LIST_ENTRIES applyFunc, void *additionalInfo);


/**
* Frees the list of strings
*/
void free_linked_list(LinkedList *pList);

/**
* Frees array of strings.
*/
void free_array_of_strings(char **arr, int elCnt);

#endif