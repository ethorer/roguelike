/*
    Everything about enemy behavior
*/

#include "game.h"

extern int newturn;
extern int player_pos_new, player_pos_old;
extern int trail[TL_CAP], tltime[TL_CAP];
extern entity_stats player;
extern level* level_current;
extern char* screen;

void epos(entity* enemy)
{
    int go = 0;
    entity* current_entity = level_current->entity_liststart;

    //Enemy movement
    if (newturn)       /* add to wait on each turn */
        ++enemy->wait;
    if (enemy->wait > enemy->speed){         /* enemies only move when their wait is over their speed, then wait resets */
        enemy->wait = 0;
        enemy->atkready = 1;                /* readies an enemy attack */
    }

    //Pathfinding
    if (newturn && (enemy->HP > 0)){     /* if new turn and enemy is not dead.. */
        make_path(enemy->path, enemy->Pold, player_pos_new);       /* make path from the enemy to the player */
        // if (obs(enemy->path) == 0)                              /* if enemy can see the player, make player position lastseen */
        //     enemy->lastseen = player_pos_new;
        if (obs(enemy->path) > 0) {                              /* if the path is obstructed.. */
            for (int i = 0; i < TL_CAP; ++i){
                int smallest = 100;
                make_path(enemy->path, enemy->Pold, trail[i]);       /* make path to each trail time element, and save the index */
                if ((obs(enemy->path) == 0) && (tltime[i] <= smallest)){            /* of the most recent one in line of sight */
                    smallest = tltime[i];
                    go = i;
                    //enemy->lastseen = trail[go];
                }
                //screen[1] = '0' + go;
            }
            make_path(enemy->path, enemy->Pold, trail[go]);      /* make path to the trail element whose index we just saved */
            // if (obs(enemy->path) == 0)                          /* if enemy can see it, save it as lastseen */
            //     enemy->lastseen = trail[go];
            // if (obs(enemy->path) > 0)                           /* if enemy can't see it, make path to lastseen */
            //     make_path(enemy->path, enemy->Pold, enemy->lastseen);
        }

        if (obs(enemy->path) == 0 && enemy->wait >= enemy->speed)    /* if it's time for the enemy to move, move it forward on the path we just made */
            enemy->P = enemy->path[0];
        if (obs(enemy->path) > 0 && enemy->idle > 8){                /* if the path is still obstructed and it's time for idle movement, do idle movement */
            enemy->P = rand_pos(enemy->P);
            enemy->idle = 0;                                         /* reset idle timer to zero if it just made an idle movement */
        }
            if (
                level_current->map[enemy->P] != '.' 
                && level_current->map[enemy->P] != TILE_TUNNEL 
                && level_current->map[enemy->P] != -79
                && level_current->map[enemy->P] != '>'
                && level_current->map[enemy->P] != '<'
            )                                /* keeps enemy from walking through walls*/
                enemy->P = enemy->Pold;
            enemy->idle++;                                           /* increase idle timer (for idle movement) */
    }


    if (dist(enemy->P, enemy->lastseen) <= 1)                 /* clears lastseen if enemy is adjacent to it */
        enemy->lastseen = 0;
    if (enemy->P == player_pos_new || enemy->P == 0)            /*keeps enemy from occupying same space as player or glitching to position 0 (fix this)*/
        enemy->P = enemy->Pold;
    while (current_entity != NULL){
        if (current_entity != enemy && enemy->P == current_entity->P)
            enemy->P = enemy->Pold;
        current_entity = current_entity->next;
    }

    // Enemy attack
    if ((dist(enemy->P, player_pos_new) <= 1) && enemy->atkready == 1 && enemy->HP > 0 && newturn == 1){
        int atk = (enemy->stats->atk + random(0, 3)) - player.def;
        if (atk < 0)
            atk = 0;
        else
            player.hp -= atk;
        create_announcement(0, enemy->stats->name, atk);
        enemy->atkready = 0;        /* un-readies attack */
    }
}

// Makes a path from one position (pos) to the other (target)
void make_path(int array[], int pos, int target)
{
    int Y = pos / SCREEN_WIDTH;
    int X = pos % SCREEN_WIDTH;
    int targetY = target / SCREEN_WIDTH;
    int targetX = target % SCREEN_WIDTH;
    int i = 0;

    for (int i = 0; i < 100; ++i)           
        array[i] = 0;

    for (int i = 0; (Y * SCREEN_WIDTH + X) != target; i++){   /* moves coordinates closer to target one by one and */
        if (X > targetX)                  /* keeps track of the positions along the way */
            --X;
        else if (X < targetX)
            ++X;
        if (Y > targetY)
            --Y;
        else if (Y < targetY)
            ++Y;

        array[i] = Y * SCREEN_WIDTH + X;
    }
}

// Counts obstacles (right now just # characters) in an array (a path) and returns how many it counted (val)
int obs(int array[])
{
    int i, val, range;
    range = 0;
    val =  0;

    for (int i = 0; array[i] != 0; ++i) /* finds the end of the path so that the next loop doesn't do anything unnecessary */
        range++;

    for (int i = 0; i < range; ++i)
        if (is_wall(level_current->map[array[i]])
            || level_current->map[array[i]] == ' ')   /* if there's a # in the path add to val */
            return 1;  

    return val;
}

// Finds distance between positions
int dist(int pos1, int pos2)
{
    int X1, Y1, X2, Y2;
    X1 = pos1 % SCREEN_WIDTH;
    Y1 = pos1 / SCREEN_WIDTH;
    X2 = pos2 % SCREEN_WIDTH;
    Y2 = pos2 / SCREEN_WIDTH;

    return (sqrt(pow(abs(X1 - X2), 2) + pow(abs(Y1 - Y2), 2)));
}

// Finds a random position for wandering behavior
int rand_pos(int pos)
{
    int X = pos % SCREEN_WIDTH;
    int Y = pos / SCREEN_WIDTH;

    int n = range_rand(0, 2);

    if (n == 0)         /* and if n == 2 X will not change*/
        X++;
    else if (n == 1)
        X--;

    n = range_rand(0, 2);

    if (n == 0)
        Y++;
    else if (n == 1)
        Y--;

    return (X + Y * SCREEN_WIDTH);  /* returns the new position */
}

// Used to find random numbers within a range (replaced this with macro)
int range_rand(int lower, int upper)
{
    int num = (rand() % (upper - lower + 1)) + lower;
    return num;
}


