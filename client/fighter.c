
#include "fighter.h"

#include <sys/select.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "socket.h"
#include "logging.h"
#include "dump.h"

struct mapnode_t {
    int playerid;
    int hitpoint;
};

struct player_t {
    int playerid;
    char name[64];
};

struct gameinfo_t {
    int width;
    int height;
    struct mapnode_t * gamemap;
    struct player_t players[20];
    int player_count;
    int player_iterator;
    int current_player;
    int myplayerid;
    int attack_x;
    int attack_y;
};

static socket_t sock = SOCKET_T_INITIALIZER;
static struct gameinfo_t the_game;

static char * parse_gameinfo(char * data)
{
    const int INIT = 0;
    const int INFO = 1;
    const int MAP = 2;
    const int HIT = 3;
    const int ATTACK = 4;
    const int PLAYERS = 5;

    int current_status = INIT;
    int row = 0;
    int reach_last_line = 0;

    char * line = NULL;
    char * end = data - 1;
    while (!reach_last_line) {
        line = end + 1;
        end = line;
        while (*end != '\r' && *end != '\n' && *end != '\0') {
            end++;
        }
        if (*end == '\0') {
            reach_last_line = 1;
        }
        *end = '\0';
        while (*line == ' ' || *line == '\r' || *line == '\n') {
            line++;
        }
        char * p = end - 1;
        while (*p == ' ' && p >= line) {
            *p = '\0';
            p--;
        }
        if (*line == '\0') {
            continue;
        }

        // cf_debug("line = [%s]\n", line);

        if (strcmp(line, "[info]") == 0) {
            current_status = INFO;
        } else if (strcmp(line, "[map]") == 0) {
            current_status = MAP;
            row = 0;
        } else if (strcmp(line, "[hit]") == 0) {
            current_status = HIT;
            row = 0;
        } else if (strcmp(line, "[attack]") == 0) {
            current_status = ATTACK;
        } else if (strcmp(line, "[players]") == 0) {
            current_status = PLAYERS;
            row = 0;
        } else {
            if (current_status == INFO) {
                int w = 0, h = 0;
                if (sscanf(line, "%d %d", &w, &h) != 2) {
                    cf_error("read width and height fail: %s\n", line);
                }
                the_game.width = w;
                the_game.height = h;
                the_game.gamemap = (struct mapnode_t *)malloc(sizeof(struct mapnode_t) * w * h);
            } else if (current_status == MAP) {
                char * p = line;
                int x = 0;
                while (1) {
                    int pid = 0;
                    sscanf(p, "%d", &pid);
                    the_game.gamemap[row * the_game.width + x].playerid = pid;
                    x++;
                    while (*p != ' ' && *p != '\0') {
                        p++;
                    }
                    if (*p == '\0') {
                        break;
                    }
                    p++;
                }
                row++;
            } else if (current_status == HIT) {
                char * p = line;
                int x = 0;
                while (1) {
                    int hit = 0;
                    sscanf(p, "%d", &hit);
                    the_game.gamemap[row * the_game.width + x].hitpoint = hit;
                    x++;
                    while (*p != ' ' && *p != '\0') {
                        p++;
                    }
                    if (*p == '\0') {
                        break;
                    }
                    p++;
                }
                row++;
            } else if (current_status == PLAYERS) {
                if (row >= 20) {
                    cf_warning("more than 20 players. ignore 20+\n");
                    continue;
                }
                int pid = atoi(line);
                if (pid == 0) {
                    continue;
                }
                char * p = line;
                while (*p != ':' && *p != '\0') {
                    p++;
                }
                if (*p == '\0') {
                    continue;
                }
                p++;
                while (*p == ' ') {
                    p++;
                }
                the_game.players[row].playerid = pid;
                snprintf(the_game.players[row].name, 64, "%s", p);
                row++;
                the_game.player_count = row;
            } else if (current_status == ATTACK) {
                int x, y;
                sscanf(line, "%d %d", &x, &y);
                the_game.attack_x = x;
                the_game.attack_y = y;
            } else {
                cf_warning("ignore current line: [%s]\n", line);
            }
        }
    }
    return end;
}

static char * parse_gameturn(char * data)
{
    the_game.current_player = atoi(data);
    
    return data + strlen(data);
}

static char * parse_playerid(char * data)
{
    the_game.myplayerid = atoi(data);
    cf_log("my playerid is %d\n", the_game.myplayerid);
    return data + strlen(data);
}

int fighter_init(const char * ip, int port)
{
    the_game.width = 0;
    the_game.height = 0;
    the_game.gamemap = NULL;
    the_game.player_count = 0;
    the_game.player_iterator = 0;
    the_game.current_player = 0;
    the_game.myplayerid = 0;

    sock = socket_create_tcp();
    int ret = socket_connect(sock, ip, port);
    if (ret != 0) {
        return ret;
    }
    return 0;
}

int fighter_loop(gameinfo_callback gicb, gameturn_callback gtcb)
{
    fd_set rfds;
    struct timeval tv;
    int ret;

    errno = 0;
    while (1) {
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        FD_SET(socket_fd(sock), &rfds);
        ret = select(socket_fd(sock) + 1, &rfds, NULL, NULL, &tv);
        // cf_log("%d bytes read.\n", ret);

        if (ret < 0) {
            cf_error("select fail: %d:%s\n", errno, strerror(errno));
            break;
        } else if (ret == 0) {
            continue;
        } else {
            char buffer[40960];
            ret = socket_recv(sock, buffer, sizeof(buffer) - 1);
            // cf_debug("%d bytes read.\n", ret);

            if (ret <= 0) {
                cf_error("socket_recv fail: %d:%s\n", errno, strerror(errno));
                break;
            }
            buffer[ret] = '\0';
            // printf("%s\n", buffer);

            // TODO
            char * packet = buffer;
            while (packet < buffer + ret) {
                char * p = packet;
                char * data = NULL;
                while (*p != ' ' && *p != '\n' && *p != '\r' && *p != '\0') {
                    p++;
                }
                if (*p != '\0') {
                    data = p + 1;
                    while (*data == '\r' || *data == '\n' || *data == ' ') {
                        data++;
                    }
                }
                *p = '\0';

                if (strcmp(packet, "GAME") == 0) {
                    packet = parse_gameinfo(data);
                    gicb(&the_game);
                } else if (strcmp(packet, "TURN") == 0) {
                    packet = parse_gameturn(data);
                    gtcb(&the_game, the_game.current_player);
                } else if (strcmp(packet, "PLAYERID") == 0) {
                    packet = parse_playerid(data);
                } else if (*packet == '\0') {
                    packet ++;
                } else {
                    cf_warning("unknown command: [%s]\n", packet);
                    packet = p + 1;
                }
            }
        }
    }
    return errno;
}

int fighter_login(const char * token)
{
    char message[100];
    snprintf(message, sizeof(message), "LOGIN %s\n", token);
    socket_send(sock, message, strlen(message) + 1);
    return 0;
}

int gameinfo_get_mapwidth(gameinfo info)
{
    return info->width;
}

int gameinfo_get_mapheight(gameinfo info)
{
    return info->height;
}

int gameinfo_get_attack_x(gameinfo info)
{
    return info->attack_x;
}

int gameinfo_get_attack_y(gameinfo info)
{
    return info->attack_y;
}

int fighter_attack(int x, int y)
{
    if (x < 0 || y < 0 || x >= the_game.width || y >= the_game.height) {
        return -1;
    }
    char message[100];
    snprintf(message, sizeof(message), "ATTACK %d %d\n", x, y);
    socket_send(sock, message, strlen(message) + 1);
    return 0;
}

mapnode gameinfo_get_mapnode(gameinfo info, int x, int y)
{
    int w = info->width;
    int h = info->height;
    if (x < 0 || x >= w || y < 0 || y >= h) {
        return NULL;
    }
    return &(info->gamemap[y * w + x]);
}

void gameinfo_start_player_iterator(gameinfo info)
{
    info->player_iterator = 0;
}

player gameinfo_next_player(gameinfo info)
{
    if (info->player_iterator < info->player_count) {
        info->player_iterator++;
        return &(info->players[info->player_iterator - 1]);
    }
    return NULL;
}

int mapnode_get_playerid(mapnode node)
{
    return node->playerid;
}

int mapnode_get_hitpoint(mapnode node)
{
    return node->hitpoint;
}

int gameinfo_my_playerid(gameinfo info)
{
    return info->myplayerid;
}

int player_id(player p)
{
    return p->playerid;
}

const char * player_name(player p)
{
    return p->name;
}

int gameinfo_player_count(gameinfo info)
{
    return info->player_count;
}


