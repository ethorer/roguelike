/*
    Coloring the screen, line of sight, and 
    player perspective
*/

#include "game.h"

extern int player_pos_old, player_pos_new;
extern int cursor_pos;
extern char screen_menu[SCREEN_HEIGHT * SCREEN_WIDTH - 1]; 
extern WORD colors_arr[SCREEN_HEIGHT*SCREEN_WIDTH];
extern level* level_current;
extern entity_stats player;
extern int vision[];

typedef enum 
{
    WholeWordsOff,
    WholeWordsOn
} keep_whole_words;

bool is_wall(char arg);
bool is_corner(char arg);

// puts colors of items, enemies etc in color array
void ColorScreen(int map_pos)
{
    // clear old colors
    // color entities
    // color items

    for (int i = 0; i < SCREEN_HEIGHT * SCREEN_WIDTH; i++)
        if (colors_arr[i] != COLOR_CLEAR)
            colors_arr[i] = COLOR_CLEAR;

    item_listnode* current_item;
    tile_listnode* current_tile = level_current->tile_liststart;
    entity* current_entity = level_current->entity_liststart;

    while (current_tile != NULL){
        if (current_tile->item_liststart != NULL 
            && vision[current_tile->pos] == 1)
            colors_arr[current_tile->pos - map_pos] 
            = current_tile->item_liststart->item->color;
        current_tile = current_tile->next;
    }

    while (current_entity != NULL){
        if (current_entity->HP <= 0 && vision[current_entity->P] == 1)
            colors_arr[current_entity->P - map_pos] 
            = current_entity->stats->color | BACKGROUND_RED;
        else if (vision[current_entity->P] == 1)
            colors_arr[current_entity->P - map_pos]
             = current_entity->stats->color;
        current_entity = current_entity->next;
    }

    for (int i = 0; i < 30; i++)
        colors_arr[80*24 + i]= COLOR_CLEAR | FOREGROUND_INTENSITY;

    if (player.hp <= 0)
        colors_arr[player_pos_new - map_pos] = BACKGROUND_RED;
    else
        colors_arr[player_pos_new - map_pos] = COLOR_CLEAR;

    colors_arr[cursor_pos] = COLOR_CLEAR;

}

void WriteMessageToScreen(int pos, char* str)
{
    int x, y, i;

    x = pos % SCREEN_WIDTH;
    y = pos / SCREEN_WIDTH;

   str_squash(str, 22, WholeWordsOff);

    for (int i = 0; *(str + i) != '\0'; i++){
        if (*(str + i) == '\n' || *(str + i) == '\t')
            y++, x = pos % SCREEN_WIDTH;
        else
            screen_menu[x + y * SCREEN_WIDTH] = *(str + i), x++;
    }
}

// raycasting
void los(int pos, double range, double angle, int *array)
{
    int j, dx, dy;
    double x, y, d, c, dotx, doty, dotc, dota, theta, r;

    int pathpos = pos;

    // need to fix this so it works without having different inequalities in each case
    if (angle <= 90.0)
        dx = 1, dy = TILE_TUNNEL;
    else if (angle <= 180.0)
        angle -= 90.0, dx = 1, dy = 80;
    else if (angle < 270.0)
        angle -= 180.0, dx = -1, dy = 80;
    else if (angle < 360.0)
        angle -= 270.0, dx = -1, dy = TILE_TUNNEL;

    x = 0.5, y = 0.5, dotx = doty = .01;
    d = sqrt(x*x + y*y);

    double rad_angle = angle * (3.1415 / 180.0);
    int obstacle_wall = 0;
    int obstacle_hall = 0;
    int obstacle_space = 0;
    int obstacle_floor = 0;

    while (d < range && level_current->map[pathpos] != ' ') {

        if (is_wall(level_current->map[pathpos])//level_current->map[pathpos] == '#'
            )
            obstacle_wall++;
        else if (level_current->map[pathpos] == TILE_TUNNEL)
             obstacle_hall++, obstacle_floor = 0;
        else if (level_current->map[pathpos] == ' ')
             obstacle_space++, obstacle_floor = 0;
        else if (level_current->map[pathpos] == '.')
             obstacle_floor++;

        c = acos((d*d + x*x - y*y) / (2.0*x*d));
        if (rad_angle > c)
            y+=1.0, doty+=1.0, pathpos = pathpos + dy;
        else if (rad_angle < c)
            x+=1.0, dotx+=1.0, pathpos = pathpos + dx;
        r = sqrt(dotx*dotx + doty*doty);
        dota = acos((r*r + dotx*dotx - doty*doty) / (2.0*dotx*r));
        theta = sqrt((dota - rad_angle)*(dota - rad_angle));
        if (r*theta < 0.5){ 

            array[pathpos] = 1;
            
            if (
                obstacle_wall > 0 
                && !is_wall(level_current->map[pathpos])
            )
                array[pathpos] = 0;
            
            if (
                obstacle_hall > 0 
                && level_current->map[pathpos] == TILE_TUNNEL
            )
                array[pathpos] = 0;
            
            if (
                is_wall(level_current->map[pathpos])
                && obstacle_floor == 0
            )
                array[pathpos] = 0;
        }              

        if (
            (
            is_wall(level_current->map[pathpos])
            || level_current->map[pathpos] == '<'
            || level_current->map[pathpos] == '>'
            || level_current->map[pathpos] == TILE_TUNNEL
            )
            && array[pathpos] == 1
        )
            level_current->seen[pathpos] = 1;

        if (obstacle_wall > 0)
            break;

        d = sqrt((x*x) + (y*y));
    }
}

// which portion of the map will be displayed on the screen
int MapOnScreen()
{
    static int MosX = 0;
    static int MosY = 0;

    int BoundX_L = MosX + 10;
    int BoundX_R = MosX + 46;
    int BoundY_T = MosY + 5;
    int BoundY_B = MosY + 20;

    if (player_pos_old % SCREEN_WIDTH < BoundX_L)
        MosX--, MapOnScreen();
    else if (player_pos_old % SCREEN_WIDTH > BoundX_R)
        MosX++, MapOnScreen();

    if (player_pos_old / SCREEN_WIDTH < BoundY_T)
        MosY--, MapOnScreen();
    else if (player_pos_old / SCREEN_WIDTH > BoundY_B)
        MosY++, MapOnScreen();

    return MosY * MAP_WIDTH + MosX;
}

// is arg a wall?
bool is_wall(char arg) 
{
    if (
        arg == TILE_CORNER_BR
        || arg == TILE_CORNER_TR
        || arg == TILE_CORNER_BL
        || arg == TILE_CORNER_TL
        || arg == TILE_WALL_VERT
        || arg == TILE_WALL_HORIZ
        )
        return true;
    else
        return false;
}

bool is_corner(char arg) 
{
    if (
        arg == TILE_CORNER_BR
        || arg == TILE_CORNER_TR
        || arg == TILE_CORNER_BL
        || arg == TILE_CORNER_TL
        )
        return true;
    else
        return false;
}
