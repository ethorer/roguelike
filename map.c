#include "game.h"

extern int player_pos_new, player_pos_old;
extern level* level_current;
extern entity_stats enemies[];
extern game_item items[];


// extern double places[16];

typedef struct coords {
    int y;
    int x;
} coords;

coords* draw_room(int hy, int hx, int ly, int lx, char map[SCREEN_HEIGHT][SCREEN_WIDTH]);
bool is_end_block(int y, int x, char map[SCREEN_HEIGHT][SCREEN_WIDTH]);

void go_back_level(level** current)
{
    if ((*current)->previous != NULL)
        (*current) = (*current)->previous;
}

void goForwardLevel(level** current)
{
    if ((*current)->next != NULL)
        (*current) = (*current)->next;
    else
        create_new_level(current);
}

// puts level in level list and spawns items, enemies, stairs
void create_new_level(level** current)
{
    insert_level(create_map_wrapper(), current);

    int floortiles[SCREEN_HEIGHT*SCREEN_WIDTH];

    for (int j = 0, i = 0; i < SCREEN_HEIGHT*SCREEN_WIDTH; i++){
        floortiles[i] = 0;
        (*current)->map[i] == '.' 
            ? floortiles[j++] = i 
            : NULL;
    }

    for (int i = 0; floortiles[i] != 0; ++i){
        if (floortiles[i+1] == 0) {
            int r = 0;
            while (
                (r == 0)
                || dist(floortiles[r], player_pos_new) < 10
                )
                r = random(0, i); 
                
            (*current)->map[floortiles[r]] = '>';
            for (int j = 0; j < random(1, 3); j++) {
                while (floortiles[r] == player_pos_new)
                    r = random(0, i); 
                spawn_entity(
                    floortiles[r] % SCREEN_WIDTH, 
                    floortiles[r] / SCREEN_WIDTH, 
                    &(enemies[random(0, 7)])
                );
            }
            for (int j = 0, r2 = 0; j < random(1, 4); j++){
                while (
                    (r = random(0, i)) == player_pos_new 
                    || (*current)->map[floortiles[r]] == '>'
                    || (*current)->map[floortiles[r]] == '<'
                    )
                    ; 
                r2 = random(0, 10);
                if (r2 <= 5)
                    spawn_item(floortiles[r], generate_item(&items[0]));
                else if (r2 > 5 && r2 <= 8)
                    spawn_item(floortiles[r], generate_item(&items[1]));
                else if (r2 > 8 && r2 <= 10)
                    spawn_item(floortiles[r], generate_item(&items[2]));
            }
            if ((*current)->previous == NULL)
                player_pos_new = floortiles[random(0, i)];
            else {
                //player_pos_new = floortiles[random(0, i)];
                (*current)->map[player_pos_new] = '<';
                }
            } 
    }

}


char* create_map_wrapper() // for converting map from 2d to 1d 
{
    char map[25][80];
    static int seed = 0;
    seed += time(0);
    while (!create_map(seed++, map))
        create_map(seed++, map);

    char* output = malloc(sizeof(char)*SCREEN_WIDTH*SCREEN_HEIGHT);

    for (int i = 0; i < SCREEN_HEIGHT*SCREEN_WIDTH; i++)
        output[i] = map[i / SCREEN_WIDTH][i % SCREEN_WIDTH];

    return output;
}

int create_map(int seed, char map[SCREEN_HEIGHT][SCREEN_WIDTH])
{
    srand(seed);

    for (int x = 0; x < 80; x++)
        for (int y = 0; y < 25; y++)
            map[y][x] = ' ';

    int num_of_rooms = random(3, 6); //(rand() % (7 - 3) + 3);
    int distances[num_of_rooms][num_of_rooms];
    bool connected[num_of_rooms][num_of_rooms];
    coords* centers[num_of_rooms];
    int connect_check[num_of_rooms];
    int retry = 0;

    for (int i = 0; i < num_of_rooms; i++){
        centers[i] = NULL;
        connect_check[i] = -1;
        for (int j = 0; j < num_of_rooms; j++){
            connected[i][j] = false;
            distances[i][j] = 0;
        }
    }

    for (int i = 0, tries = 0; i < num_of_rooms; ){ // tries to draw rooms, retries if it can't fit one, after 100 retries gives up
       if (centers[i] = draw_room(random(2, 18), random(5, 60), random(3, 5), random(3, 10), map)){   
            i++; 
            tries = 0;
        }
        else
            tries++;
        if (tries > 100)
            break;
    }

    for (int room = 0; room < num_of_rooms; room++) // creates distances table
        for (int i = 0; i < num_of_rooms; i++){
                if (centers[i] != NULL)
                    distances[room][i] 
                    = distances[i][room] 
                    = coord_dist(centers[room], centers[i]);
            }

    for (int room = 0; room < num_of_rooms; room++) // finds closest room to each room and attempts to draw a path -- if it can't, whole thing retries
        if (centers[room] != NULL) {
            int smallest = 100;
            int new = 0;
            int target = 0;
            for (int i = 0; i < num_of_rooms; i++){
                if ((centers[i] != NULL) 
                    && (i != room) 
                    && (connected[i][room] == false)){
                        if ((new = distances[room][i]) < smallest)
                            smallest = new, target = i;
                }
            }
            if (smallest == 100)
                ;
            else {
            if (!coord_make_path(centers[room], centers[target], map))
                retry = 1;
            //coord_make_path(centers[room], centers[target]);
            connected[room][target] = true;
            connected[target][room] = true;
            }
        }

    int c = 0;

    connect_check[0] = 0;
    
    for (int i = 0; i < num_of_rooms; i++) // checks if number of connected rooms is equal to actual number of rooms -- if not, retry
        if (connect_check[i] != -1){
            for (int j = 0; j < num_of_rooms; j++)
                if (connected[connect_check[i]][j])
                    for (int k = 0; k < num_of_rooms; k++){
                        if (connect_check[k] == j)
                            break;
                        if (k+1 == num_of_rooms)
                            connect_check[++c] = j;
                    }
        }

    int nullct1 = 0, nullct2 = 0;

    for (int i = 0; i < num_of_rooms; i++)
        if (centers[i] == NULL)
            nullct1++;

    for (int i = 0; i < num_of_rooms; i++)
        if (connect_check[i] == -1)
            nullct2++;

    if (nullct1 != nullct2){
        map[1][1] = 'x';//'0' + nullct2;
        retry = 1;
    }

    if (level_current != NULL)
        {
            if (map[player_pos_new/SCREEN_WIDTH][player_pos_new%SCREEN_WIDTH] != '.')   
                retry = 1;
        }

    if (retry)
        return 0;
    else 
        return 1;
}

coords* draw_room(int hy, int hx, int ly, int lx, char map[SCREEN_HEIGHT][SCREEN_WIDTH])
{
    coords* center = malloc(sizeof(coords)); 

    for (int y = hy; y <= hy + ly; y++) // for spacing between rooms
            for (int x = hx; x <= hx + lx; x++)
                for (int wx = -2; wx <= 2; wx++)
                    for (int wy = -2; wy <= 2; wy++)
                        if (map[y+wy][x+wx] != ' ')
                            return NULL;

    for (int y = hy; y <= hy + ly; y++) // long due to fancy walls
        for (int x = hx; x <= hx + lx; x++){
            map[y][x] = '.';
            if ((x == hx) && (y == hy))
                map[y-1][x] = 0xFFCD, map[y-1][x-1] = 0xFFC9, map[y][x-1] = 0xFFBA;
            else if ((x == hx + lx) && (y == hy))
                map[y-1][x] = 0xFFCD, map[y-1][x+1] = 0xFFBB, map[y][x+1] = 0xFFBA;
            else if ((x == hx) && (y == hy + ly))
                map[y][x-1] = 0xFFBA, map[y+1][x-1] = 0xFFC8, map[y+1][x] = 0xFFCD;
            else if ((x == hx + lx) && (y == hy + ly))
                map[y][x+1] = 0xFFBA, map[y+1][x+1] = 0xFFBC, map[y+1][x] = 0xFFCD;
            else
                {
                if (map[y-1][x] != '.')
                    map[y-1][x] = 0xFFCD;
                if (map[y+1][x] != '.')
                    map[y+1][x] = 0xFFCD;
                if (map[y][x-1] != '.')
                    map[y][x-1] = 0xFFBA;
                if (map[y][x+1] != '.')
                    map[y][x+1] = 0xFFBA;
                }
        }
    
    center->y = hy + ly/2;
    center->x = hx + lx/2;

    //map[hy + ly/2][hx + lx/2] = 'o';
    return center;
}

int coord_dist(coords* a, coords* b)
{
    return sqrt((a->x - b->x)*(a->x - b->x)
        + (a->y - b->y)*(a->y - b->y));
}

int coord_make_path(coords* a, coords* b, char map[SCREEN_HEIGHT][SCREEN_WIDTH])
{
    int pathy = a->y;
    int pathx = a->x;
    int pathy_old = a->y;
    int pathx_old = a->x;
    int plus_or_minus = 1;
    int thresholds[10];
    for (int i = 0; i < 10; i++)
        thresholds[i] = 0;
    int i = 0;

    while ((pathx != b->x) || (pathy != b->y)){   
 
        if (random(0, 10) <= 5) // randomly moves toward target
        {    
            if (pathx < b->x)
                pathx++;
            else if (pathx > b->x)
                pathx--;
        }
        else
        {    
            if (pathy < b->y)
                pathy++;
            else if (pathy > b->y)
                pathy--;
        }

        retry:
        if (map[pathy][pathx] == ' ')
            map[pathy][pathx] = TILE_TUNNEL;
        else if (map[pathy][pathx] == '.')
            ;
        else if (map[pathy][pathx] == TILE_WALL_HORIZ) // horiz.
        {
            if (map[pathy][pathx+1] == TILE_TUNNEL
                || map[pathy][pathx-1] == TILE_TUNNEL)
                {
                    pathy = pathy_old;
                    pathx = pathx_old;

                    if (random(0, 10) <= 5) 
                    {    
                        if (random(0, 10) <= 5)
                            pathx++;
                        else 
                            pathx--;
                    }
                    else
                    {    
                        if (random(0, 10) <= 5)
                            pathy++;
                        else 
                            pathy--;
                    }
                    
                    goto retry;
                }
            else {
                map[pathy][pathx] = TILE_TUNNEL;
                thresholds[i++] = pathy*SCREEN_WIDTH + pathx;
                }
        }
        else if (map[pathy][pathx] == TILE_WALL_VERT) // vert.
        {
            if (map[pathy+1][pathx] == TILE_TUNNEL
                || map[pathy-1][pathx] == TILE_TUNNEL)
                {
                    pathy = pathy_old;
                    pathx = pathx_old;
                    if (random(0, 10) <= 5) 
                    {    
                        if (random(0, 10) <= 5)
                            pathx++;
                        else 
                            pathx--;
                    }
                    else
                    {    
                        if (random(0, 10) <= 5)
                            pathy++;
                        else 
                            pathy--;
                    }
                    goto retry;
                }
            else{
                map[pathy][pathx] = TILE_TUNNEL;
                thresholds[i++] = pathy*SCREEN_WIDTH + pathx;
                }
        }            
        else if (
            (map[pathy][pathx] == TILE_CORNER_TL) //0xFFC9) // top left
            || (map[pathy][pathx] == TILE_CORNER_TR)//0xFFBB) // top right
            || (map[pathy][pathx] == TILE_CORNER_BL)//0xFFC8) // btm left
            || (map[pathy][pathx] == TILE_CORNER_BR)//0xFFBC) // btm right
            ) 
                return 0;
        

        pathy_old = pathy;
        pathx_old = pathx;
    }

    for (i = 0; i < 10; i++)
        {
            int x = thresholds[i] % SCREEN_WIDTH;
            int y = thresholds[i] / SCREEN_WIDTH;

            if (map[y+1][x] == ' '
                || is_end_block(y+1, x, map))
                //if (map[y-1][x] == '.')
                    map[y][x] = TILE_WALL_HORIZ;
            if (map[y-1][x] == ' '
                || is_end_block(y-1, x, map))
                //if (map[y+1][x] == '.')
                    map[y][x] = TILE_WALL_HORIZ;
            if (map[y][x+1] == ' '
                || is_end_block(y, x+1, map))
                //if (map[y][x-1] == '.')
                    map[y][x] = TILE_WALL_VERT;
            if (map[y][x-1] == ' '
                || is_end_block(y, x-1, map))
                //if (map[y][x+1] == '.')
                    map[y][x] = TILE_WALL_VERT;
        }

    return 1;
}

bool is_end_block(int y, int x, char map[SCREEN_HEIGHT][SCREEN_WIDTH])
{
    int count = 0;

    if (map[y+1][x] == ' ') 
        count++;
    if (map[y-1][x] == ' ') 
        count++;
    if (map[y][x+1] == ' ') 
        count++;
    if (map[y][x-1] == ' ') 
        count++;

    if (count >= 3)
        return true;
    else
        return false;
}


