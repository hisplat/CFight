
#include "game.h"

#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "logging.h"
#include "client.h"


static void on_client_login(client_t * client, char * arg);
static void on_client_attack(client_t * client, char * arg);

static struct {
    const char * command;
    void (* handler)(client_t * client, char * arg);
} command_table[] = {
    { "LOGIN", on_client_login },
    { "ATTACK", on_client_attack },
    { NULL, NULL },
};

typedef struct {
    int playerid;
    int hitpoint;
} mapnode;
static mapnode gamemap[MAP_HEIGHT][MAP_WIDTH];

static void on_client_login(client_t * client, char * token)
{
    cf_log("on client login by token: %s.\n", token);
    player_t * p = find_player_by_token(token);
    if (p != NULL) {
        client->player = p;
        client->status = CLIENT_GAME;
        cf_log("player '%s' joined game.\n", p->name);
    }
}

static void on_client_attack(client_t * client, char * arg)
{
}


void read_client_data(client_t * client)
{
    char buffer[1024];
    int ret = socket_recv(client->socket, buffer, sizeof(buffer) - 1);
    buffer[ret] = '\0';
    cf_debug("%d bytes read.", ret);
    if (ret <= 0) {
        char ip[100];
        cf_log("[%s:%d] disconnected.\n", socket_get_remote_ip(client->socket, ip, sizeof(ip)), socket_get_remote_port(client->socket));
        client_destroy(client);
        return;
    }

    char * p = buffer;
    char * arg = NULL;
    while (*p != ' ' && *p != '\0') {
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
        }
    }
}

int main_loop(int port)
{
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

        tv.tv_sec = 1;
        tv.tv_usec = 0;

        int ret = select(maxfd + 1, &rfds, NULL, NULL, &tv);
        if (ret < 0) {
            if (errno == EINTR) {
                continue;
            }
            cf_error("will exit server.");
            break;
        } else if (ret == 0) {
            continue;
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
            }
        }
    }
    return 0;
}

