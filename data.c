/*
    Data for items and enemies.
*/

#include "game.h"

//spd, hp, atk, def
entity_stats enemies[] = {
    {"chimp", 'm', 2, 4, 5, 0, FOREGROUND_GREEN},
    {"soldier", '@', 3, 6, 5, 0, FOREGROUND_RED | FOREGROUND_INTENSITY},
    {"snake", 's', 4, 6, 5, 0, FOREGROUND_RED},
    {"magician", '@', 3, 6, 5, 0, FOREGROUND_BLUE},
    {"scarab", 0xFFE9, 2, 6, 5, 0, FOREGROUND_GREEN | FOREGROUND_INTENSITY},
    {"sphinx", 'L', 2, 6, 5, 0, FOREGROUND_GREEN | FOREGROUND_BLUE},
    {"troll", 'T', 3, 6, 5, 0, FOREGROUND_RED | FOREGROUND_INTENSITY}
};

//name, icon, atk, def, color, fun
game_item items[] = {
    {"health potion", 0xFFAD, 0, 0, FOREGROUND_BLUE | FOREGROUND_INTENSITY, health_restore_wrapper},
    {"sword (+3) (+1)", '/', 3, 1, FOREGROUND_RED | FOREGROUND_GREEN, NULL},
    {"axe (+2) (+2)", '/', 2, 2, FOREGROUND_GREEN, NULL}
};
