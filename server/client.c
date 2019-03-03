

#include "client.h"

#include <stdlib.h>
#include <string.h>


static client_t * client_head = NULL;
static client_t * client_tail = NULL;
static client_t * current_client = NULL;

struct client_iterator_t {
    client_t * current;
};

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
    c->player = NULL;
    c->status = CLIENT_INIT;
    c->speed_left = 0;
    c->in_turn = 0;
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

client_t * find_client_by_player(player_t * p)
{
    client_t * c = client_head;
    while (c != NULL) {
        if (c->player == p) {
            return c;
        }
        c = c->next;
    }
    return NULL;
}

client_iterator client_begin_iterator()
{
    struct client_iterator_t * iterator = (struct client_iterator_t*)malloc(sizeof(struct client_iterator_t));
    iterator->current = client_head;
    return iterator;
}

client_t * client_iterator_next(client_iterator iter)
{
    client_t * c = iter->current;
    if (iter->current != NULL) {
        iter->current = iter->current->next;
    }
    return c;
}

void client_end_iterator(client_iterator iter)
{
    free(iter);
}



