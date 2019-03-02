
#pragma once

#include "socket.h"
#include "player.h"

#define CLIENT_INIT 0
#define CLIENT_GAME 1

typedef struct __client_t {
    socket_t socket;
    player_t * player;

    int status;

    struct __client_t * next;
    struct __client_t * prev;
} client_t;


void client_init();
client_t * client_create();
void client_destroy(client_t *c);

void client_reset_iterator();
client_t * client_next();

