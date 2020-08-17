#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include "Utilities.h"

char* concat(const char *s1, const char *s2)
{
    char *result = malloc(strlen(s1) + strlen(s2) + 1); // +1 for the null-terminator
                                                        // in real code you would check for errors in malloc here
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

void free_variables(int count, ...)
{
    va_list vl;
    int i;

    va_start(vl, count);    // init argument list

    for (i = 0; i < count; i++)
    {
        free(va_arg(vl, void *)); // free each argument
    }

    va_end(vl); // clean up
}

LinkedList* init_linked_list()
{
    LinkedList* pList = (LinkedList*)malloc(sizeof(LinkedList));
    pList->head = NULL;
    return pList;
}

Node* create_empty_node()
{
    Node *entry = (Node *)malloc(sizeof(Node));
    if (entry)
    {
        entry->data = NULL;
        entry->next = NULL;
    }
    return entry;
}

BOOLEAN add_entry_to_list(LinkedList* pList, void* data, size_t dataLengthInByte)
{
    Node *curNode = pList->head;
    Node *entry = create_empty_node();

    if (!entry)
    {
        printf("No memory to create entry in list of strings\n");
        return FALSE;
    }
    entry->data = malloc(dataLengthInByte); 
    if (!entry->data)
    {
        printf("No memory to create entry in list of strings\n");
        free(entry);
        return FALSE;
    }
    memcpy(entry->data, data, dataLengthInByte);
    if (!curNode)   // first entry
    {
        pList->head = entry;
    }
    else
    {
        while (curNode->next)
        {
            curNode = curNode->next;
        }
        curNode->next = entry;
    }
    return TRUE;
}

void apply_to_linked_list(LinkedList* pList, FUNCTION_TO_APPLY_ON_LINKED_LIST_ENTRIES applyFunc, void* additionalInfo)
{
    Node *pCur = pList->head;
    while (pCur)
    {
        applyFunc(pCur, additionalInfo);
        pCur = pCur->next;
    }
}

void free_linked_list(LinkedList *pList)
{
    Node *curNode = NULL, *nextNode = NULL;
    if (pList)
    {
        curNode = pList->head;
        while (curNode)
        {
            nextNode = curNode->next;
            free(curNode);
            curNode = nextNode;
        }
        free(pList);
    }
}

void free_array_of_strings(char** arr, int elCnt)
{
    int i;
    if (elCnt > 0)
    {
        for (i = 0; i < elCnt; i++)
        {
            free(arr[i]);
        }
        free(arr);
    }
}