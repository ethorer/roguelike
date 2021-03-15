#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <windows.h>
#include <stdbool.h>

#define MAP_WIDTH 80
#define MAP_HEIGHT 25
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25
#define TL_CAP 30   
#define TL_DECAY 30  
#define COLOR_CLEAR FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN
#define _WIN32_WINNT 0x0500

//for map
#define TILE_TUNNEL -80
#define TILE_WALL_HORIZ -51
#define TILE_WALL_VERT -70
#define TILE_CORNER_TL -55
#define TILE_CORNER_TR -69
#define TILE_CORNER_BL -56
#define TILE_CORNER_BR -68

#define greatest(a, b) (a >= b ? a : b)
#define random(a, b) (rand() % (b - a) + a)
#define b(a) (int) places[a]

// linked list for items
typedef struct game_item
{
    char* item_name;
    char icon;
    int atk;
    int def;
    DWORD color;
    void (*function)(); 
} game_item;

typedef struct item_listnode
{
    int item_ID;
    game_item* item;
    struct item_listnode* next;
    struct item_listnode* previous;
} item_listnode;

typedef struct tile_listnode
{
    int pos;
    item_listnode* item_liststart;
    struct tile_listnode* next;
} tile_listnode;

typedef struct announcement {
    char* str;
    struct announcement* next;
    struct announcement* previous;
} announcement;

typedef struct entity_stats
{
    char* name;
    char icon;
    int speed;
    int hp;
    int atk;
    int def;
    DWORD color;
} entity_stats;

typedef struct entity
{
    int P, Pold;
    int X, Y, Xold, Yold;
    int HP, speed;
    int wait, pathpos, obs, lastseen, atkready, idle;
    int path[100];

    entity_stats* stats;
    struct entity* next;
} entity;

typedef struct level
{
    char* map;
    char* seen;
    tile_listnode* tile_liststart;
    entity* entity_liststart;
    struct level* next;
    struct level* previous;
} level;

void insert_item(game_item* x, item_listnode** start);
void insert_item_inventory(item_listnode**, item_listnode*);
void delete_item(item_listnode** start, int);
void spawn_item(int pos, item_listnode* item);
char* get_item_names(int pos);
item_listnode* generate_item(game_item* item);
void spawn_entity(int X, int Y, entity_stats* stats);
void create_new_level();
char* create_map_wrapper();
void effect_wrapper1();
void epos(entity* enemy);
void make_path(int array[], int pos, int target);
int obs(int array[]);
int dist(int pos1, int pos2);
int rand_pos(int pos);
int range_rand(int lower, int upper);
void fowlos(int pos, int range, int *array);
void create_menu(int pos, char* text);
char* inv_update(int, char*);
void str_squash(char* str, int width, int);
//char* create_map();//char map_arr[], int map_width, int map_height);
int draw_r(int x1, int y1, int x2, int y2, int arr[]);
int charcount(char*, char);
char* get_position_names(int);
entity* entity_at_position(int);
int hud_state(int);
void hud_update(int);
void cursor_posFun(int);
void hotkeys(int);
void go_back_level();
void insert_level(char*, level**);
item_listnode* select_item_from_menu(int, item_listnode*);
void create_announcement(int arg, char* name, int val);
//void CullAnnouncements();
bool is_wall(char arg);
void melee_attack(entity* target);
void health_restore();
void health_restore_wrapper();
