
#include "fighter.h"
#include "logging.h"
#include <stdio.h>
#include <algorithm>
#include <map>

static int start_x = -1;
static int start_y = -1;

void update_gameinfo(gameinfo info)
{
#if 0
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
#endif
}

void update_gameturn(gameinfo info, int current_playerid)
{
    int me = gameinfo_my_playerid(info);
    if (current_playerid != me) {
        return;
    }

    std::map<std::pair<int, int>, int> locs;

    int w = gameinfo_get_mapwidth(info);
    int h = gameinfo_get_mapheight(info);

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            mapnode node = gameinfo_get_mapnode(info, x, y);
            int playerid = mapnode_get_playerid(node);
            if (playerid == me) {
                if (start_x == -1) {
                    start_x = x;
                    start_y = y;
                }
                mapnode up = gameinfo_get_mapnode(info, x, y - 1);
                mapnode down = gameinfo_get_mapnode(info, x, y + 1);
                mapnode left = gameinfo_get_mapnode(info, x - 1, y);
                mapnode right = gameinfo_get_mapnode(info, x + 1, y);
                if (up != NULL && mapnode_get_playerid(up) != me) {
                    locs[std::pair<int, int>(x, y - 1)] = mapnode_get_hitpoint(up);
                }
                if (down != NULL && mapnode_get_playerid(down) != me) {
                    locs[std::pair<int, int>(x, y + 1)] = mapnode_get_hitpoint(down);
                }
                if (left != NULL && mapnode_get_playerid(left) != me) {
                    locs[std::pair<int, int>(x - 1, y)] = mapnode_get_hitpoint(left);
                }
                if (right != NULL && mapnode_get_playerid(right) != me) {
                    locs[std::pair<int, int>(x + 1, y)] = mapnode_get_hitpoint(right);
                }
            }
        }
    }

    int nearest = -1;
    std::pair<int, int> loc;

    std::for_each(locs.begin(), locs.end(), [&](std::pair<const std::pair<int, int>, int> &iter) {
        int x = iter.first.first;
        int y = iter.first.second;
        int distance = (x - start_x) * (x - start_x) + (y - start_y) * (y - start_y);
        if (nearest == -1 || distance < nearest) {
            nearest = distance;
            loc = iter.first;
        }
    });

    cf_debug("nearest = %d, loc = (%d, %d)\n", nearest, loc.first, loc.second);
    fighter_attack(loc.first, loc.second);
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

