

#include "client.h"

#include <stdlib.h>
#include <string.h>


static client_t * client_head = NULL;
static client_t * client_tail = NULL;
static client_t * current_client = NULL;


void client_init()
{
    client_head = NULL;
    client_tail = NULL;
    current_client = NULL;
}

client_t * client_create()
{
    client_t * c = (client_t*)malloc(sizeof(client_t));

    c->socket = SOCKET_T_INITIALIZER;
    c->speed = 0;
    c->attack = 0;
    c->hit = 0;
    c->next = NULL;
    c->prev = NULL;
    if (client_head == NULL) {
        client_head = c;
    }
    if (client_tail != NULL) {
        client_tail->next = c;
        c->prev = client_tail;
    }
    client_tail = c;
    return c;
}

void client_destroy(client_t *c)
{
    client_t * prev = c->prev;
    client_t * next = c->next;

    if (prev != NULL) {
        prev->next = next;
    } else {
        client_head = next;
    }
    if (next != NULL) {
        next->prev = prev;
    } else {
        client_tail = prev;
    }
    if (current_client == c) {
        current_client = next;
    }
    free(c);
}

void client_reset_iterator()
{
    current_client = client_head;
}

client_t * client_next()
{
    client_t * t = current_client;
    if (current_client != NULL) {
        current_client = current_client->next;
    }
    return t;
}


