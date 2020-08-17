#ifndef __UTILITIES__
#define __UTILITIES__

#define RETURN_ON_MEMORY_FAILURE(pointerToCheck, valToReturn) \
            if (!(pointerToCheck)) \
            { \
                printf("Failed to allocate memory, exiting..."); \
                return (valToReturn); \
            }
            
#define GOTO_LABEL_ON_NULL(pointerToCheck, label, statusVar, statusToSet, message) \
            if (!pointerToCheck) \
            { \
                statusVar = statusToSet; \
                printf(message); \
                goto label; \
            }

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