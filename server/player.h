
#pragma once


typedef struct {
    char name[64];
    char token[32];
    int speed;
    int attack;
    int hit;
    int id;
} player_t;

void load_all_players();

player_t * find_player_by_token(const char * token);
player_t * find_player_by_id(int id);



