
#include <stdio.h>
#include <stdlib.h>
#include "dqueue.h"

typedef struct dqnode DQNode;

struct dqueue
{
    DQNode *begin;
    DQNode *end;
};

struct dqnode
{
    int client_socket;
    DQNode *next;
};

Queue *queue_create()
{
    Queue *qu;
    qu = malloc(sizeof(Queue));
    if (qu)
    {
        qu->begin = NULL;
        qu->end = NULL;
    }
    return qu;
}

void queue_free(Queue *qu)
{
    if (qu != NULL)
    {
        DQNode *p;
        while (qu->begin != NULL)
        {
            p = qu->begin;
            qu->begin = qu->begin->next;
            free(p);
        }
        free(qu);
    }
}

/*Adiciona um item no fim da fila q*/
int queue_push(Queue *qu, int client_socket)
{
    if (!qu)
        return INVALID_NULL_POINTER;
    DQNode *node;
    node = malloc(sizeof(DQNode));
    if (node == NULL)
        return OUT_OF_MEMORY;
    node->client_socket = client_socket;
    node->next = NULL;
    if (queue_empty(qu))
    {
        qu->begin = node;
        qu->end = node;
    }
    else
    {
        qu->end->next = node;
    }

    qu->end = node;

    return SUCCESS;
}

/*Remove um item do início da fila e retorna o client*/
int queue_pop(Queue *qu, int* client_socket)
{
    if (!qu)
        return INVALID_NULL_POINTER;
    if (queue_empty(qu))
        return ELEM_NOT_FOUND;

    DQNode *p;
    p = qu->begin;
    *client_socket = p->client_socket;
    qu->begin = p->next;

    if (qu->begin == NULL)
        qu->end = NULL;

    free(p);
    return SUCCESS;
}

int queue_empty(Queue *qu)
{
    if (!qu)
        return INVALID_NULL_POINTER;
    return (qu->begin == NULL);
}
