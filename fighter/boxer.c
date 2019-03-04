
#include "fighter.h"
#include "logging.h"

#include <stdio.h>

void update_gameinfo(gameinfo info)
{
    int w = gameinfo_get_mapwidth(info);
    int h = gameinfo_get_mapheight(info);

    cf_log("map size = (%d x %d)\n", w, h);

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            mapnode node = gameinfo_get_mapnode(info, x, y);
            int playerid = mapnode_get_playerid(node);
            int hitpoint = mapnode_get_hitpoint(node);
            printf("%d(%d) ", playerid, hitpoint);
        }
        printf("\n");
    }
    
    printf("player count = %d\n", gameinfo_player_count(info));
    gameinfo_start_player_iterator(info);
    player p = NULL;
    while ((p = gameinfo_next_player(info)) != NULL) {
        int pid = player_id(p);
        const char * pname = player_name(p);
        cf_log("player: %d (%s)\n", pid, pname);
    }
}

void update_gameturn(int playerid)
{
}

int main()
{
    if (fighter_init("localhost", 11233) != 0) {
        return -1;
    }
    fighter_login("TOKEN_BOXER");
    fighter_loop(update_gameinfo, update_gameturn);

    return 0;
}

