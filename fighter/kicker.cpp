
#include "fighter.h"
#include "logging.h"
#include <stdio.h>
#include <algorithm>
#include <map>



void update_gameinfo(gameinfo info)
{
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

    srand((unsigned int)time(NULL));
    int r = (rand() % locs.size());

    std::map<std::pair<int, int>, int>::const_iterator iter;
    for (iter = locs.begin(); iter != locs.end() && r > 0; iter++, r--) {
    }

    cf_debug("attack (%d, %d)\n", iter->first.first, iter->first.second);
    fighter_attack(iter->first.first, iter->first.second);

}


int main()
{
    if (fighter_init("localhost", 11233) != 0) {
        return -1;
    }
    fighter_login("TOKEN_KICKER");
    fighter_loop(update_gameinfo, update_gameturn);

    return 0;
}

