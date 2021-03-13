#include "game.h"

extern int player_pos_old, player_pos_new;
extern int newturn;
extern item_listnode* wielded_items[3];
extern struct entity_stats player;
extern char screen_menu[SCREEN_HEIGHT * SCREEN_WIDTH - 1]; 
extern int cursor_pos;
extern int map_pos;
extern level* level_current;

item_listnode* spells_start;
item_listnode* inv_start;

typedef enum 
{
    WholeWordsOff,
    WholeWordsOn
} keep_whole_words;

// mostly for selecting things from menus
void menus(int state)
{
    item_listnode* current_item;
    tile_listnode* current;

    switch(state)
    {
        case 1 : // pick up items
            current = level_current->tile_liststart;
            while (current != NULL){
                if (current->pos == player_pos_old){
                   current_item = current->item_liststart;
                    if (current_item != NULL){
                        if (current_item->next != NULL){
                            int item_number = wait_player_input(0x47) - 0x40;
                            for (; item_number > 1; item_number--){
                                if (current_item == NULL)
                                    return;
                                current_item = current_item->next;
                            }
                        }
                            delete_item(&(current->item_liststart), current_item->item_ID);
                            create_announcement(2, current_item->item->item_name, 0);//, display_announcements();
                            insert_item_inventory(&inv_start, current_item);
                            newturn = 1;
                    }
                }
                current = current->next;
                hud_update(0), hud_state(0);
            }
            break;
        case 2 : // drop items
            current_item = select_item_from_menu(0x44, inv_start);
            if (current_item != NULL) {
                delete_item(&(inv_start), current_item->item_ID);
                spawn_item(player_pos_old, current_item);
                if (wielded_items[0])
                    if (current_item->item_ID == wielded_items[0]->item_ID)
                        wielded_items[0] = NULL;
                if (wielded_items[1])
                    if (current_item->item_ID == wielded_items[1]->item_ID)
                        wielded_items[1] = NULL;
                newturn = 1;
                hud_update(0), hud_state(0);
            }
            break;
        case 3 : // view inventory
            wait_player_input(0x49);
            break;
        case 5 : // consume
            current_item = select_item_from_menu(0x45, inv_start);
            if (current_item != NULL) {
                if (current_item->item->function != NULL) {
                    current_item->item->function();
                    delete_item(&inv_start, current_item->item_ID);
                    newturn = 1;
                }
                if (wielded_items[0])
                    if (current_item->item_ID == wielded_items[0]->item_ID){
                        hud_update(0), hud_state(0); 
                        return;
                    }
                if (wielded_items[1])
                    if (current_item->item_ID == wielded_items[1]->item_ID){
                        hud_update(0), hud_state(0); 
                        return;
                    }
                }
            // else
            //     create_announcement(4, NULL, 0);
            break;
        case 6 : // spells
            current_item = select_item_from_menu(0x43, spells_start);
            if (current_item){
                current_item->item->function();
                newturn = 1;
            }
            break;
        case 7 : // wield
            current_item = select_item_from_menu(87, inv_start);   
            if (current_item){                  // probably could be better
                if (!wielded_items[0]){
                    if (wielded_items[1]){
                        if (wielded_items[1]->item_ID == current_item->item_ID)
                            wielded_items[1] = NULL;
                        else
                            wielded_items[0] = current_item;
                    }
                    else
                        wielded_items[0] = current_item;
                }
                else if (wielded_items[0]){   //unwields
                    if (wielded_items[0]->item_ID == current_item->item_ID)
                        wielded_items[0] = NULL;
                    else
                        wielded_items[0] = current_item;
                }

                if (wielded_items[1])   
                    if (wielded_items[1]->item_ID == current_item->item_ID)
                        wielded_items[1] = NULL; 

                hud_update(0), hud_state(0);
            }
            break;

    }
}

item_listnode* select_item_from_menu(int hold, item_listnode* start)
{
    int item_number = wait_player_input(hold) - 0x40;
    if (item_number < 0){
        hud_update(0), hud_state(0); 
        return NULL;
    } 
    item_listnode* current_item = start;
    if (current_item == NULL){
        hud_update(0), hud_state(0); 
        return NULL;
    }
    for (; item_number > 1; item_number--){
        current_item = current_item->next;
        if (current_item == NULL){
            hud_update(0), hud_state(0); 
            return NULL;
        }
    }
    return current_item;
}

// not used
void createmenu(int pos, char* text)
{
    int i, j, X, Y;

    i = j = X = Y = 0;

    int height = 2;
    int width = 0;

    if (pos != -1) {
        str_squash(text, 50, WholeWordsOn);

        // floating menu
        for(X = i = 0; text[i] != '\0'; i++){    // this one writes in the text
            if (text[i] == '\n')
                (X > width) ? width = X : 0, Y++, X = 0, height++;
            else if (text[i] == '\t')
                (X > width) ? width = X : 0, Y++, X = 3, height++;
            else
                screen_menu[(pos % SCREEN_WIDTH) + 1 + (X++) + SCREEN_WIDTH * (Y+1)] = text[i];
        }

        if (width > 50){
            width = 50;
        }

        width++;
    }

    // side menu

    else {
        pos = SCREEN_WIDTH - 22;

        if (pos == SCREEN_WIDTH - 22)
            width = 21, height = 24;

        for(X = i = 0; text[i] != '\0'; i++){    // this one writes in the text
            if (text[i] == '\n')
                Y++, X = 0;
            else if (X >= width - 2)
                screen_menu[(pos % SCREEN_WIDTH) + 1 + (X++) + SCREEN_WIDTH * (Y+1)] = '-', Y++, X = 5, i--;
            else
                screen_menu[(pos % SCREEN_WIDTH) + 1 + (X++) + SCREEN_WIDTH * (Y+1)] = text[i];
        }
    }

    for (Y = pos / SCREEN_WIDTH; Y < (pos / SCREEN_WIDTH) + height + 1; Y++)         /* this loop draws the border */
            for (X = (pos % SCREEN_WIDTH); X < (pos % SCREEN_WIDTH) + width + 1; X++)
                    if (Y == pos / SCREEN_WIDTH || Y == (pos / SCREEN_WIDTH) + height)
                        screen_menu[X + Y * SCREEN_WIDTH] = 0xFFCD;
                    else if (X == (pos % SCREEN_WIDTH) || X == (pos % SCREEN_WIDTH) + width)
                        screen_menu[X + Y * SCREEN_WIDTH] = 0xFFBA;
                    else if (screen_menu[X + Y * SCREEN_WIDTH] == '~')
                        screen_menu[X + Y * SCREEN_WIDTH] = ' ';
    screen_menu[pos] = 0xFFC9;
    screen_menu[pos + width] = 0xFFBB;
    screen_menu[pos + height * SCREEN_WIDTH] = 0xFFC8;
    screen_menu[pos + height * SCREEN_WIDTH + width] = 0xFFBC;
    Y = (pos / SCREEN_WIDTH) + 1;

    free(text);
}

// for word wrapping
void str_squash(char* str, int width, int keep_whole_words)
{
    int i, j, last_space;
    i = j = last_space = 0;

    if (keep_whole_words == WholeWordsOn)
        for (i = 0; *(str + i) != '\0'; i++){
            if (*(str + i) == '\n')
                j = 0;
            else if (*(str + i) == ' ')
                last_space = i, j++;
            else if (j >= width)
                *(str + last_space) = '\t', j = 0;
            else
                j++;
        }
    else if (keep_whole_words == WholeWordsOff)
        for (i = 0; *(str + i) != '\0'; i++){
            if (*(str + i) == '\n')
                j = 0;
            else if (j >= width)
                *(str + i) = '\t', j = 0;
            else
                j++;
        }

}

// shows inventory items with a header
char* invUpdate(int arg, char* header)
{
    static char* text = NULL;
    if (text != NULL)
        free(text);
    item_listnode* current;

    if (arg == 0)
        current = inv_start;
    else 
        current = spells_start;

    int sz = strlen(header) + 3;
    char selector[5] = " ) ";
    char selector_code = 'a';

    for(; current != NULL; current = current->next)
        sz += strlen(current->item->item_name) + 5;

    text = (char*) malloc(sizeof(char) * (strlen(header) + sz + 1));
    strcpy(text, header);
    strcat(text, "\n\n");

    if (arg == 0)
        current = inv_start;
    else 
        current = spells_start;

    while (current != NULL){
        selector[0] = selector_code++;
        strcat(text, selector);
        strcat(text, current->item->item_name);
        if (wielded_items[0] != NULL)
            if (current->item_ID == wielded_items[0]->item_ID)
                strcat(text, " (W)");
        if (wielded_items[1] != NULL)
            if (current->item_ID == wielded_items[1]->item_ID)
                strcat(text, " (W)");
        strcat(text, "\n");

        current = current->next;
    }

    return text;
}

// gets items at map position
char* GetPositionNames(int pos)
{
    item_listnode* current_item = NULL;
    tile_listnode* current_tile = level_current->tile_liststart;
    entity* current_entity = level_current->entity_liststart;
    static char* str = NULL;
    if (str != NULL)
        free(str);
    str = malloc(250*sizeof(char));
    memset(str, '\0', sizeof(str));
    strcpy(str, "\n");

    while (current_entity != NULL && current_entity->P != pos)
        current_entity = current_entity->next;

    if (current_entity != NULL){
        strcat(str, current_entity->stats->name);
        strcat(str, "\n");
    }

    while (current_tile != NULL && current_tile->pos != pos)
        current_tile = current_tile->next;

    if (current_tile != NULL)
        current_item = current_tile->item_liststart;

    while (current_item != NULL){
        strcat(str, current_item->item->item_name);
        strcat(str, "\n");
        current_item = current_item->next;
    }

    return str;
}

// initializes hud
void HudCreate()
{
    int x, y;

    for (y = 0; y <= 25; y++){
        screen_menu[56 + y*80] = 0xFFB3;
    }

    for (x = 57; x < 80; x++)
        for (y = 0; y <= 25; y++)
            screen_menu[x + y*80] = ' ';

    hud_update(0);

//    for (x = 0; x < 10; x++)
//        announcements[x] = " ";

}

// returns which menu is open on the hud
int hud_state(int arg)
{
    static int state = 0;

    if (state == arg)
        state = 0;
    else if (arg >= 0)
        state = arg;

    return state;
}

void HudClear() 
{
    for (int x = 57; x < 80; x++)
        for (int y = 0; y <= 25; y++)
            screen_menu[x + y*80] = ' ';
}

// puts the menus on the screen
void hud_update(int arg)
{
    char* str = NULL;
    if (str != NULL)
        free(str);
    str = malloc(sizeof(char) * 200);
    memset(str, '\0', sizeof(str));

    switch (arg){
    case 0 : // default hud
        sprintf(str, "\n %s\n\n HP: %d / 10\n AP: %d / 20\n\n W1: %s\n W2: %s", 
            "Placeholder", 
            player.hp, 
            20, 
            (wielded_items[0] != NULL) ? wielded_items[0]->item->item_name : NULL, 
            (wielded_items[1] != NULL) ? wielded_items[1]->item->item_name : NULL
            );
        break;
    case 1 : 
        strcpy(str, "What would you like to pick up?\n\n");
        char* items_get = get_item_names(player_pos_new);
        if (items_get == NULL)
            return;
        strcat(str, items_get);
        if (charcount(str, '\n') <= 3)
            return;
        break;
    case 2 : 
        strcpy(str, invUpdate(0, "What would you like to drop?\n\n"));
        break;
    case 3 : 
        strcpy(str, invUpdate(0, "Inventory:\n\n"));
        break;
    case 4 : // looking around
        0;
        char* items_look = GetPositionNames(cursor_pos + map_pos);
        if (items_look != NULL)
            strcpy(str, items_look);
        break;
    case 5 :
        strcpy(str, invUpdate(0, "What would you like to use/consume?\n\n"));
        break;
    case 6 :
        strcpy(str, invUpdate(1, "Which spell would you like to cast?\n\n"));
        break;
    case 7 :
        strcpy(str, invUpdate(0, "What would you like to wield?\n\n"));
        break;
    }

    HudClear();

    WriteMessageToScreen(57, str);
}

void Target()
{
    PlayerMovement(wait_player_input(0x4C), &cursor_pos);
}

// toggles cursor on or off ("on" is greater than -1)
void cursor_posFun(int offset)
{
    static int inc = 1;

    cursor_pos = -1 + (player_pos_new - offset + 1) * (inc++ % 2);
}

// attacks entity at a position
void AttackPosition(int pos)
{
    entity* entity = EntityAtPosition(pos);

    if (entity != NULL)
        entity->HP--, create_announcement(1, entity->stats->name, 1);
}

// returns entity at position
entity* EntityAtPosition(int pos)
{
    entity* current_entity = level_current->entity_liststart;

    while (current_entity != NULL && current_entity->P != pos)
        current_entity = current_entity->next;

    if (current_entity != NULL){
        return current_entity;
    }
    return NULL;
}

int charcount(char* str, char c)
{
    int count = 0;

    for(int i = 0; *(str + i) != '\0'; i++)
        if (*(str + i) == c)
            count++;

    return count;
}
