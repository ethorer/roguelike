/*
    List nodes for the various linked lists in the game, and some
    functions for manipulating them
*/

#include "game.h"

extern level* level_current;

void spawn_item(int pos, item_listnode* item)
{
    tile_listnode* current = level_current->tile_liststart;

    item->next = item->previous = NULL;

    // if the tile node already exists
    while (current != NULL){
        if (current->pos == pos){
            current->item_liststart != NULL ? current->item_liststart->previous = item : 0;
            item->next = current->item_liststart;
            current->item_liststart = item;
            return;
        }
        else
            current = current->next;
    }

    // if the tile node doesn't exist
    tile_listnode* tile_temp;

    tile_temp = (tile_listnode*) malloc(sizeof(tile_listnode));

    tile_temp->pos = pos;

    tile_temp->next = level_current->tile_liststart;
    tile_temp->item_liststart = item;

    level_current->tile_liststart = tile_temp;

}

void insert_level(char* map, level** current)
{
    //level* current = level_current;
    
    level* temp = malloc(sizeof(level));
    temp->next = temp->previous = NULL;
    temp->map = map;
    char* seen = calloc(SCREEN_WIDTH*SCREEN_HEIGHT, sizeof(char));
    temp->seen = seen;
    temp->entity_liststart = NULL;
    temp->tile_liststart = NULL;

    if (*current == NULL){
        *current = temp;
        return;
    }

    while (*current != NULL){
        if ((*current)->next == NULL){
            (*current)->next = temp;
            (*current)->next->previous = (*current);
            (*current) = (*current)->next;
            return;
        }
        else 
            *current = (*current)->next;
    }
}

char* get_item_names(int pos)
{
    tile_listnode* current = level_current->tile_liststart;
    item_listnode* current_list;
    int output_length = 0;
    char* output = NULL;
    char selector_code = 0;
    char selector_text[5] = "  - ";

    if (output != NULL)
        free(output);

    if (current == NULL){
        return NULL;
        }

    while (current != NULL){
        if (current->pos == pos){
            current_list = current->item_liststart;
            // if (current_list == NULL){
            //     output = malloc(sizeof(char)*2);
            //     strcpy(output, "\0");
            //     return output;
            // }
                
            while (current_list != NULL){
                output_length += 1 + strlen(current_list->item->item_name);
                current_list = current_list->next;
            }
            output = malloc(sizeof(char)*output_length+50);
            strcpy(output, "\0");
            current_list = current->item_liststart;
            while (current_list != NULL){
                selector_text[0] = 'a' + selector_code++;
                strcat(output, selector_text);
                strcat(output, current_list->item->item_name);
                strcat(output, "\n");
                current_list = current_list->next;
            }
        }
        current = current->next;
    }
    return output;
}

// creates an item_listnode for an item
item_listnode* generate_item(game_item* item)
{
    static int global_ID = 0;
    item_listnode* output = (item_listnode*) malloc(sizeof(item_listnode));

    output->item_ID = global_ID++;
    output->item = item;
    output->next = NULL;
    output->previous = NULL;

    return output;
}

void insert_item_inventory(item_listnode** start, item_listnode* item)
{

    item->next = item->previous = NULL;

    if (*start == NULL){
        *start = item;
        return;
    }

    item->next = *start;
    (*start)->previous = item;
    *start = item;

}

// deletes an item from some list
void delete_item(item_listnode** item, int item_ID)
{
    while (((*item)->item_ID != item_ID) && ((*item) != NULL))
        item = &((*item)->next);
    if ((*item)->item_ID == item_ID){
        if((*item)->previous == NULL){
            if ((*item)->next == NULL)
                (*item) = NULL;
            else {
            (*item)->next->previous = NULL;
            *item = (*item)->next;
            }
        }
        else if ((*item)->next == NULL)
            (*item)->previous->next = (*item)->next;
        else{
            (*item)->next->previous = (*item)->previous;
            (*item)->previous->next = (*item)->next;
        }
    }
}

// spawns a unique entity given stats
void spawn_entity(int X, int Y, entity_stats* stats)
{
    entity* temp = (entity*) calloc(1, sizeof(entity));

    temp->P = temp->Pold = Y * SCREEN_WIDTH + X;
    temp->Xold = temp->X = X;
    temp->Yold = temp->Y = Y;

    temp->stats = stats;
    temp->HP = stats->hp;
    temp->speed = stats->speed;

    if (level_current->entity_liststart == NULL){
        temp->next = NULL;
        level_current->entity_liststart = temp;
        return;
    }

    temp->next = level_current->entity_liststart;
    level_current->entity_liststart = temp;
}
