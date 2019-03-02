
#pragma once

#include "socket.h"

typedef struct __client_t {
    socket_t socket;
    int speed;
    int attack;
    int hit;

    struct __client_t * next;
    struct __client_t * prev;
} client_t;


void client_init();
client_t * client_create();
void client_destroy(client_t *c);

void client_reset_iterator();
client_t * client_next();

