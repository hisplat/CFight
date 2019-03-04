
#include "game.h"

#include <sys/select.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "logging.h"
#include "client.h"
#include "dump.h"


static void on_client_login(client_t * client, char * arg);
static void on_client_attack(client_t * client, char * arg);
static void on_client_start(client_t * client, char * arg);

static struct {
    const char * command;
    void (* handler)(client_t * client, char * arg);
} command_table[] = {
    { "LOGIN", on_client_login },
    { "ATTACK", on_client_attack },
    { "START", on_client_start },
    { NULL, NULL },
};

typedef struct {
    int playerid;
    int hitpoint;
} mapnode;
static mapnode gamemap[MAP_HEIGHT][MAP_WIDTH];
static int gamemap_changed = 0;
static int game_seed = 0;
static int current_game_time = 0;

static client_t * current_client = NULL;
static int need_reset_current_client = 0;
static int current_client_timeout = 0;

#define GAME_STATUS_INIT 0
#define GAME_STATUS_PLAYING 1
static int game_status = GAME_STATUS_INIT;

////////////

static void reset_current_client()
{
    current_client = NULL;
    client_iterator iter = client_begin_iterator();
    client_t * c = NULL;
    while ((c = client_iterator_next(iter)) != NULL) {
        if (c->in_turn) {
            current_client = c;
            break;
        }
    }
    client_end_iterator(iter);
    current_client_timeout = ACTION_TIMEOUT_TICK;

    if (current_client != NULL) {
        cf_log("[gametime: %d] now '%s' is in turn.\n", current_game_time, current_client->player->name);
    } else {
        cf_log("[gametime: %d] now no one is in turn.\n", current_game_time);
    }
    need_reset_current_client = 0;
}

static void broadcast_gameturn()
{
    if (current_client == NULL) {
        return;
    }

    char message[100];
    snprintf(message, sizeof(message), "TURN %d\n\n", current_client->player->id);

    client_reset_iterator();
    client_t * client = NULL;
    while ((client = client_next()) != NULL) {
        socket_send(client->socket, message, strlen(message) + 1);
    }
}

static void init_game_map()
{
    srand(game_seed);
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            gamemap[y][x].playerid = 0;
            gamemap[y][x].hitpoint = 0;
        }
    }

    client_iterator iter = client_begin_iterator();
    client_t * c = NULL;
    while ((c = client_iterator_next(iter)) != NULL) {
        if (c->player != NULL) {
            while (1) {
                int y = (rand() % MAP_HEIGHT);
                int x = (rand() % MAP_WIDTH);
                if (gamemap[y][x].playerid == 0) {
                    gamemap[y][x].playerid = c->player->id;
                    gamemap[y][x].hitpoint = c->player->hit;
                    cf_log("player '%d:%s' respawn at (%d, %d)\n", c->player->id, c->player->name, x, y);
                    break;
                }
            }
        }
    }
    client_end_iterator(iter);

    gamemap_changed = 1;
}

static void broadcast_gamemap()
{
    char message[4096] = {0};

    strcpy(message + strlen(message), "GAME\n");
    sprintf(message + strlen(message), "[info]\n%d %d\n", MAP_WIDTH, MAP_HEIGHT);

    sprintf(message + strlen(message), "[map]\n");
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            sprintf(message + strlen(message), "%d ", gamemap[y][x].playerid);
        }
        sprintf(message + strlen(message), "\n");
    }

    sprintf(message + strlen(message), "[hit]\n");
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            sprintf(message + strlen(message), "%d ", gamemap[y][x].hitpoint);
        }
        sprintf(message + strlen(message), "\n");
    }

    sprintf(message + strlen(message), "[players]\n");

    client_t * client = NULL;
    client_reset_iterator();
    while ((client = client_next()) != NULL) {
        if (client->player == NULL) {
            continue;
        }
        snprintf(message + strlen(message), sizeof(message) - strlen(message), "%d: %s\n", client->player->id, client->player->name);
    }
    snprintf(message + strlen(message), sizeof(message) - strlen(message), "\n");

    client_reset_iterator();
    while ((client = client_next()) != NULL) {
        socket_send(client->socket, message, strlen(message) + 1);
    }

    gamemap_changed = 0;
}


static void on_client_login(client_t * client, char * token)
{
    cf_log("on client login by token: %s.\n", token);
    if (game_status != GAME_STATUS_INIT) {
        cf_log("game is running, cannot join game.\n");
        return;
    }
    player_t * p = find_player_by_token(token);
    if (p != NULL) {
        client_t * c = find_client_by_player(p);
        if (c == NULL) {
            client->player = p;
            client->status = CLIENT_GAME;
            cf_log("player '%s' joined game.\n", p->name);
            gamemap_changed = 1;
        } else {
            cf_error("Not implemented yet.\n");
        }
    }
}

static void on_client_attack(client_t * client, char * arg)
{
}

static void on_client_start(client_t * client, char * arg)
{
    game_status = GAME_STATUS_PLAYING;
    init_game_map();
    reset_current_client();
}

static void on_game_tick()
{
    if (game_status == GAME_STATUS_INIT) {
        // game not started yet.
        return;
    }

    if (current_client != NULL) {
        current_client_timeout --;
        cf_debug("timeleft %d tick.\n", current_client_timeout);
        if (current_client_timeout <= 0) {
            char ip[100];
            cf_warning("client [%s:%d|%s] timeout of game time %d.\n", 
                socket_get_remote_ip(current_client->socket, ip, sizeof(ip)),
                socket_get_remote_port(current_client->socket),
                current_client->player->name,
                current_game_time);
            current_client->in_turn = 0;
            need_reset_current_client = 1;
        }
        return;
    }

    cf_debug("game tick: %d\n", current_game_time);
    current_game_time++;
    client_t * client = NULL;
    client_reset_iterator();
    while ((client = client_next()) != NULL) {
        if (client->player == NULL) {
            continue;
        }
        client->speed_left += client->player->speed;
        cf_debug("player '%s' speed left = %d\n", client->player->name, client->speed_left);

        if (client->speed_left > GAME_SPEED) {
            client->speed_left -= GAME_SPEED;
            client->in_turn = 1;
            need_reset_current_client = 1;
        } else {
            client->in_turn = 0;
        }
    }

}

void read_client_data(client_t * client)
{
    char buffer[1024];
    int ret = socket_recv(client->socket, buffer, sizeof(buffer) - 1);

    // dump(buffer, ret);
    buffer[ret] = '\0';
    cf_debug("%d bytes read.", ret);
    if (ret <= 0) {
        char ip[100];
        cf_log("[%s:%d] disconnected.\n", socket_get_remote_ip(client->socket, ip, sizeof(ip)), socket_get_remote_port(client->socket));
        if (client->in_turn) {
            need_reset_current_client = 1;
        }
        client_destroy(client);
        return;
    }

    char * p = buffer;
    char * arg = NULL;
    while (*p != ' ' && *p != '\0' && *p != '\r' && *p != '\n') {
        p++;
    }
    if (*p == ' ') {
        arg = (p + 1);
        while (arg[strlen(arg) - 1] == '\n' || arg[strlen(arg) - 1] == '\r') {
            arg[strlen(arg) - 1] = '\0';
        }
    }
    *p = '\0';

    cf_debug("command = [%s], arg = [%s]\n", buffer, arg);

    for (int i = 0; command_table[i].command != NULL; i++) {
        if (strcmp(command_table[i].command, buffer) == 0) {
            command_table[i].handler(client, arg);
            cf_debug("done.");
        }
    }
}

int main_loop(int port, unsigned int seed)
{
    game_seed = seed;
    client_init();
    load_all_players();

    socket_t main_socket = socket_create_tcp();
    int mainfd = socket_fd(main_socket);

    int ret = socket_bind_and_listen(main_socket, port);
    if (ret != 0) {
        cf_error("bind to port %d failed: %d, %s\n", port, -ret, strerror(-ret));
        return -1;
    }
    cf_log("listening on port %d\n", port);

    long long timeout = get_uptime_millis() + GAME_TICK;

    while (1) {
        fd_set  rfds;
        struct timeval tv;
        client_t * client;

        FD_ZERO(&rfds);
        FD_SET(mainfd, &rfds);
        int maxfd = mainfd;

        client_reset_iterator();
        while ((client = client_next()) != NULL) {
            int fd = socket_fd(client->socket);
            FD_SET(fd, &rfds);
            if (fd > maxfd) {
                maxfd = fd;
            }
        }

        int ret = 0;
        long long to = timeout - get_uptime_millis();
        if (to > 0) {
            tv.tv_sec = to / 1000;
            tv.tv_usec = (to % 1000);
            // cf_debug("timeout = %lld\n", to);
            ret = select(maxfd + 1, &rfds, NULL, NULL, &tv);
            // cf_debug("ret = %d\n", ret);
        }

        long long now = get_uptime_millis();
        if (now >= timeout) {
            on_game_tick();
            timeout = now + GAME_TICK;
        }

        if (ret < 0) {
            // if (errno == EINTR) {
            //     continue;
            // }
            cf_error("will exit server.");
            break;
        } else if (ret == 0) {
            // continue;
        } else {
            client_reset_iterator();
            while ((client = client_next()) != NULL) {
                int fd = socket_fd(client->socket);
                if (FD_ISSET(fd, &rfds)) {
                    read_client_data(client);
                }
            }

            // accept new connection.
            if (FD_ISSET(mainfd, &rfds)) {
                socket_t s = socket_accept(main_socket);
                client_t * c = client_create();
                c->socket = s;

                char ip[100];
                cf_log("[%s:%d] connected.", socket_get_remote_ip(s, ip, sizeof(ip)), socket_get_remote_port(s));
                gamemap_changed = 1;
            }
        }

        if (need_reset_current_client) {
            reset_current_client();
            broadcast_gameturn();
        }

        if (gamemap_changed) {
            broadcast_gamemap();
        }
    }
    return 0;
}

