
#pragma once

#define MAP_WIDTH 30
#define MAP_HEIGHT 30
#define GAME_TICK 200
#define GAME_SPEED 100
#define GROW_SPEED 50
#define MAX_PARTIES 6
#define ACTION_TIMEOUT_TICK 3
#define HITPOINT_LIMIT 200
#define MAX_GAME_TIME 10000

int main_loop(int port, unsigned int seed);

