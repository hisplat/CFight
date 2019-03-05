
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mapnode_t * mapnode;
typedef struct gameinfo_t * gameinfo;
typedef struct player_t * player;

typedef void (*gameinfo_callback)(gameinfo info);
typedef void (*gameturn_callback)(gameinfo info, int current_playerid);

int fighter_init(const char * ip, int port);
int fighter_login(const char * token);
int fighter_loop(gameinfo_callback gicb, gameturn_callback gtcb);
int fighter_attack(int x, int y);


int gameinfo_get_mapwidth(gameinfo info);
int gameinfo_get_mapheight(gameinfo info);
int gameinfo_my_playerid(gameinfo info);
mapnode gameinfo_get_mapnode(gameinfo info, int x, int y);

void gameinfo_start_player_iterator(gameinfo info);
player gameinfo_next_player(gameinfo info);
int gameinfo_player_count(gameinfo info);

int mapnode_get_playerid(mapnode node);
int mapnode_get_hitpoint(mapnode node);

int player_id(player p);
const char * player_name(player p);


#ifdef __cplusplus
}
#endif

