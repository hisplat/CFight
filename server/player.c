
#include "player.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "logging.h"

static player_t * players[100];
static int total_players = 0;

static void load_player_from_file(const char * filename)
{
    FILE * fp = fopen(filename, "rb");
    if (fp == NULL) {
        return;
    }
    player_t * p = (player_t *)malloc(sizeof(player_t));
    fgets(p->token, sizeof(p->token), fp);
    p->token[strlen(p->token) - 1] = '\0';
    fgets(p->name, sizeof(p->name), fp);
    p->name[strlen(p->name) - 1] = '\0';
    fscanf(fp, "%d %d %d", &(p->speed), &(p->attack), &(p->hit));
    fclose(fp);

    p->id = total_players + 1;
    players[total_players] = p;
    total_players++;

    cf_log("player '%d:%s(%d, %d, %d)' loaded.\n", p->id, p->name, p->speed, p->attack, p->hit);
}

void load_all_players()
{
    total_players = 0;
    FILE* fp = fopen("./data/player/list", "rb");
    if (fp == NULL) {
        return;
    }
    while (!feof(fp)) {
        char filename[100];
        if (fgets(filename, sizeof(filename), fp) == NULL) {
            continue;
        }
        while (filename[strlen(filename) - 1] == '\r' || filename[strlen(filename) - 1] == '\n') {
            filename[strlen(filename) - 1] = '\0';
        }
        char * p = filename;
        while (*p == ' ' || *p == '\t') {
            p++;
        }
        if (*p == '\0') {
            continue;
        }
        char file[1024];
        snprintf(file, sizeof(file), "./data/player/%s", p);
        load_player_from_file(file);
    }
    fclose(fp);
}


player_t * find_player_by_token(const char * token)
{
    for (int i = 0; i < total_players; i++) {
        if (strcmp(players[i]->token, token) == 0) {
            return players[i];
        }
    }
    return NULL;
}

player_t * find_player_by_id(int id)
{
    for (int i = 0; i < total_players; i++) {
        if (players[i]->id == id) {
            return players[i];
        }
    }
    return NULL;
}

