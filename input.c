#include "game.h"

extern int player_pos_new;
extern int key_hold;
extern int cursor_pos;
extern int map_pos;
extern level* level_current;
extern int newturn, player_pos_new, player_pos_old;
extern char map_world[SCREEN_WIDTH*SCREEN_HEIGHT - 1];
extern entity_stats player;
extern item_listnode* wielded_items[3];

enum keyCodes {
    keyA = 0x41, keyB, keyC, keyD, keyE, keyF, keyG, keyH, keyI, keyJ, 
    keyK, keyL, keyM, keyN, keyO, keyP, keyQ, keyR, keyS, keyT, keyU,
    keyV, keyW, keyX, keyY, keyZ

};

void hotkeys(int VK) 
{

    if (key_hold == 1) // && khtest == 0)
        switch(VK){
            // case keyA :
            //     cursor_posFun(map_pos);
            //     hud_state(7);
            //     if (cursor_pos == -1)
            //         hud_update(0), hud_state(0);
            //     break;
            // case keyC :
            //     hud_update(6), hud_state(6);
            //     break;
            case keyD :
                hud_update(2), hud_state(2);
                break;
            case keyE :
                hud_update(5), hud_state(5);
                break;
            case keyG :
                hud_update(1), hud_state(1);
                break;
            case keyI :
                hud_update(3), hud_state(3);
                break;
            case keyL :
                cursor_posFun(map_pos);
                hud_state(4);
                if (cursor_pos == -1)
                    hud_update(0), hud_state(0);
                break;
            case keyS : // swaps wielded items
                wielded_items[2] = wielded_items[0]; // wielded_items[2] is temp storage
                wielded_items[0] = wielded_items[1];
                wielded_items[1] = wielded_items[2];
                if (wielded_items[0] != NULL){
                    player.atk = wielded_items[0]->item->atk;
                    player.def = 5 + wielded_items[0]->item->def; // get rid of 5
                }
                hud_update(0), hud_state(0);
                newturn = 1;
                break;
            case keyW :
                hud_update(7), hud_state(7);
                break;
        };
}

void PlayerMovement(int key_press_new, int* position)
{
    static int key_hold = 0;
    static int key_press = 0;
    int x = 0;

    if (key_press_new == key_press)
        key_hold++;
    else{
        key_press = key_press_new;
        key_hold = 0;
    }

    switch(key_press){
        case VK_NUMPAD1 :
            x = SCREEN_WIDTH - 1;
            break;
        case VK_NUMPAD2 :
            x = SCREEN_WIDTH;
            break;
        case VK_NUMPAD3 :
            x = SCREEN_WIDTH + 1;
            break;
        case VK_NUMPAD4 :
            x = -1;
            break;
        case VK_NUMPAD6 :
            x = 1;
            break;
        case VK_NUMPAD7 :
            x = -SCREEN_WIDTH - 1;
            break;
        case VK_NUMPAD8 :
            x = -SCREEN_WIDTH;
            break;
        case VK_NUMPAD9 :
            x = -SCREEN_WIDTH + 1;
            break;
        case VK_DECIMAL :
            key_hold < 1 ? newturn = 1 : 0;
            return;
        case VK_LEFT : 
            x = -1;
            break;
        case VK_RIGHT : 
            x = 1;
            break;
        case VK_UP :
            x = -SCREEN_WIDTH;
            break; 
        case VK_DOWN : 
            x = SCREEN_WIDTH;
            break;
            
     }

    if ((key_hold < 1) || (key_hold > 15)) // >15 in case you are holding it
        *position += x;

    if (level_current->map[player_pos_new] != '.'
        && level_current->map[player_pos_new] != TILE_TUNNEL) 
        if (level_current->map[player_pos_new] != '>'
        && level_current->map[player_pos_new] != '<') // keeps you from walking into walls or enemies
            player_pos_new = player_pos_old;
}

// infinite loops until you press something
int wait_player_input(int hold)
{
    int toggle = 0;
    int i = 0;

    if (hold != NULL)
        while(GetAsyncKeyState(hold));

    while(toggle == 0){
        i++ >= 0x87 ? i = 0 : 0;
        toggle = GetAsyncKeyState(i);
        key_hold++;
    }

    return i;
}

// just goes through once
int PlayerInput()
{
    for(int i = 0x24; i < 256; i++)
        if (GetAsyncKeyState(i) != 0)
            return i;

    return 0;
}
