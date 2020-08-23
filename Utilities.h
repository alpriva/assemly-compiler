#ifndef __UTILITIES__
#define __UTILITIES__

#define PRINT_NO_MEMORY_ERR() printf("Failed to allocate memory in file: %s. Line %d.\n", __FILE__, __LINE__);

#define RETURN_ON_MEMORY_FAILURE(pointerToCheck, valToReturn) \
            if (!(pointerToCheck)) \
            { \
                PRINT_NO_MEMORY_ERR(); \
                return (valToReturn); \
            }

#define PRINT_WARN(lineCnt, ...) { \
                printf("Warning: In line %d. ", lineCnt); \
                printf(__VA_ARGS__); \
                printf("\n"); \
            }
            
#define PRINT_ERR(lineCnt, ...) { \
                printf("Error: In line %d. ", lineCnt); \
                printf(__VA_ARGS__); \
                printf("\n"); \
            }
            
#define GOTO_LABEL_ON_NULL(pointerToCheck, label, statusVar, statusToSet, lineCnt, ...) \
            if (!pointerToCheck) \
            { \
                statusVar = statusToSet; \
                PRINT_ERR(lineCnt, __VA_ARGS__) \
                goto label; \
            }

#define GOTO_LABEL_ON_STATUS_ERR(statusToCheck, label, statusVar, statusToSet, lineCnt, message) \
            GOTO_LABEL_ON_NULL(statusToCheck, label, statusVar, statusToSet, lineCnt, message)

#define RETURN_STATUS_ON_ERR(valueToReturn, lineCnt, ...) \
            PRINT_ERR(lineCnt, __VA_ARGS__) \
            return valueToReturn;

typedef enum
{
    FALSE,
    TRUE
} BOOLEAN;

/**
* Creates a new string and concats the s1 and s2 strings to it. 
* The caller responsibility is to release the allocated memory.
*/
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
* Creates an array of strings
*/
char** create_array_of_strings(int elCnt);


/**
* Frees array of strings.
*/
void free_array_of_strings(char **arr, int elCnt);

#endif