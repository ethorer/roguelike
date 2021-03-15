#include "game.h"

int VK = 0;                         // holds virtual key code 
int key_hold = 0;                   // 1 if key is held
int cursor_pos = -1;                // 'look' cursor pos
int map_pos = 0;                    // where the top-left corner of the screen is on the map 
int player_pos_old = 0, 
    player_pos_new = 0;
int tl = 0;                         /*tl is trail length */
int newturn = 0;//in main            /* for determining if it's a new turn */
int trail[TL_CAP], tltime[TL_CAP];    /* trail stuff for pathfinding */

void (*queued_effect)() = NULL;
level* level_current = NULL;
item_listnode* wielded_items[3];

int vision[SCREEN_HEIGHT * SCREEN_WIDTH - 1]; /* what player can see */
char screen_menu[SCREEN_HEIGHT * SCREEN_WIDTH - 1];
char scr[SCREEN_WIDTH * SCREEN_HEIGHT - 1];
char map[SCREEN_WIDTH * SCREEN_HEIGHT - 1]; 
WORD colors_arr[SCREEN_HEIGHT*SCREEN_WIDTH];

//spd, hp, atk, def
entity_stats player = {"player", '@', 1, 10, 0, 5, FOREGROUND_INTENSITY};

int main()
{
    // initialize screen
    char* screen; 
    COORD coordstart; 
    COORD attr_coord;
    screen = scr; 
    HANDLE hConsole = CreateConsoleScreenBuffer(
        GENERIC_READ | GENERIC_WRITE, 
        0, 
        NULL,
        CONSOLE_TEXTMODE_BUFFER, 
        NULL);
    SetConsoleActiveScreenBuffer(hConsole);
    DWORD dwBytesWritten = 0;
    LPDWORD lpNumberOfAttrsWritten = 0;
    coordstart.X = coordstart.Y = 0;

    COORD bufsize = {(short) 80, (short) 25};
    SetConsoleScreenBufferSize(
        hConsole,
        bufsize
    );

    // cursor
    HANDLE out = hConsole;
    CONSOLE_CURSOR_INFO     cursorInfo;
    GetConsoleCursorInfo(out, &cursorInfo);
    cursorInfo.dwSize = 1;
    cursorInfo.bVisible = FALSE; // set the cursor visibility
    SetConsoleCursorInfo(out, &cursorInfo);

    wielded_items[0] = NULL;
    wielded_items[1] = NULL;
    wielded_items[2] = NULL;

    create_new_level(&level_current);

    //Initialize player trail to starting position
    for (int i = 0; i < TL_CAP; ++i)
       trail[i] = player_pos_old;

    //Initialize player vision and inventory menu
    for (int i = 0; i < SCREEN_HEIGHT * SCREEN_WIDTH; ++i){
       level_current->seen[i] = 0;
       screen_menu[i] = '~';      /* tildes aren't printed */
    }

    entity* current_entity = level_current->entity_liststart;

    HudCreate();

    for (int i = 0; i < SCREEN_HEIGHT * SCREEN_WIDTH; i++){
        map[i] = level_current->map[i];
        colors_arr[i] = COLOR_CLEAR;
    }

    // Game loop
    while(1)
    {
        Sleep(30);

        newturn = 0;

        // gets player input
        if(!(VK = PlayerInput()))
            key_hold = 0;
        else
            key_hold++;

        // provisional controls for stairs
        if (GetAsyncKeyState(0x10)){
            if (GetAsyncKeyState(0xBE) 
                && level_current->map[player_pos_new] == '>') {
                goForwardLevel(&level_current);
                for (int i = 0; i < TL_CAP; i++) 
                    trail[i] = 0, tltime[i] = 0;
                    }
            else if (GetAsyncKeyState(0xBC) 
                    && level_current->map[player_pos_new] == '<') {
                go_back_level(&level_current);
                for (int i = 0; i < TL_CAP; i++) 
                    trail[i] = 0, tltime[i] = 0;
			}
        }

        // look cursor -- "else" triggers an effect if one is queued
        if (cursor_pos != -1){
            if (GetConsoleWindow() == GetForegroundWindow())
                PlayerMovement(VK, &cursor_pos);
            hud_update(4);
            if (GetAsyncKeyState(VK_RETURN) && (hud_state(-1) != 4)) 
                newturn = 1, queued_effect(), queued_effect = NULL, 
                cursor_posFun(map_pos), hud_update(0), hud_state(0);
        }
        else {
            if (queued_effect) {
                queued_effect();
                queued_effect = NULL;
                hud_update(0), hud_state(0);
            }
            if (GetConsoleWindow() == GetForegroundWindow())
                PlayerMovement(VK, &player_pos_new);
        }

        // sets newturn if you moved
        if (player_pos_old != player_pos_new)
            newturn = 1;

        // context menus
        menus(hud_state(-1));

        // hotkeys
        if (key_hold == 1 && (GetConsoleWindow() == GetForegroundWindow()))
            hotkeys(VK);

        if (wielded_items[0]){
            player.atk = wielded_items[0]->item->atk;
            player.def = 5 + wielded_items[0]->item->def;
        }

        // simple attack function
        while (current_entity) {
            if (player_pos_new == current_entity->Pold 
                && current_entity->HP > 0)
                melee_attack(current_entity), player_pos_new = player_pos_old;
            current_entity = current_entity->next;
        }
        current_entity = level_current->entity_liststart;

        // Updates the player trail for enemy pathfinding
        if (newturn)       
        {

            //updates the age of every trail element (they are deleted
            // when their ages reach TL_DECAY) 
            for (int i = 0; i < TL_CAP; i++)    
                ++tltime[i];

            //if the current position is already in the trail, 
            //reset the time on that position
            for (int i = 0; i < TL_CAP; ++i)    
                if (player_pos_new == trail[i])
                    trail[i] = tltime[i] = 0;

            trail[tl] = player_pos_new; /*sets current player position to trail position*/
            tltime[tl] = 0;             /* resets the time on that position */
            tl++;                       /* increment trail position */
            if (tl >= TL_CAP)           /* if it reaches the end of the array, start over */
                tl = 0;

        }

        // Deletes expired trail elements
        for (int i = 0; i < TL_CAP; i++){
            if (tltime[i] >= TL_DECAY)
                trail[i] = 0;
          }
        

        // Enemy behavior 
        while (current_entity){
            epos(current_entity);
            current_entity = current_entity->next;
        }
        current_entity = level_current->entity_liststart;

        // Fog of war / line of sight

        for (int i = 0; i < SCREEN_HEIGHT * SCREEN_WIDTH - 1; ++i)
            vision[i] = 0;  /* resets vision array */

        vision[player_pos_new] = 1;

        for (double di = 0.0; di <= 360.0; di += 1.0)
            los(player_pos_new, 10.0, di, vision);    /* creates vision array */

        for (int i = 0; i < SCREEN_HEIGHT * SCREEN_WIDTH - 1; i++){
            map[i] = level_current->map[i]; // level_current->map is like map_world
        }

        // for (int i = 0; i < TL_CAP; i++)
        //     map[trail[i]] = tltime[i] + '0'; // makes pathfinding trail visible

        //if (level_current->map[player_pos_new] == TILE_TUNNEL)
            for (int x = -1; x <= 1; x++)   // 1-tile vision
                for (int y = -1; y <= 1; y++) {
                    int pos = player_pos_new + SCREEN_WIDTH * y + x;
                    if (level_current->map[pos] == TILE_TUNNEL) {
                        map[pos] = -79; // lighter tunnel tile
                        vision[pos] = 1;
                        level_current->seen[pos] = 1;
                    }
                    else if (level_current->map[player_pos_new] == '<')
                        if (is_wall(level_current->map[pos])) 
                            level_current->seen[pos] = 1;
                }

        // Put items on screen
        if (level_current->tile_liststart){
            tile_listnode* current = level_current->tile_liststart;
            while (current){
                if (current->item_liststart){
                    item_listnode* current_item = current->item_liststart;
                    map[current->pos] = current_item->item->icon;
                }
                current = current->next;
            }
        }

        // Put enemies on screen
        while (current_entity){
            if (vision[current_entity->P] == 1)
                map[current_entity->P] = current_entity->stats->icon;
            current_entity = current_entity->next;
        }
        current_entity = level_current->entity_liststart;

        // Copy map to screen array
        map_pos = MapOnScreen();

        ColorScreen(map_pos);
        WriteConsoleOutputAttribute(hConsole, &colors_arr, 
            SCREEN_HEIGHT*SCREEN_WIDTH - 1, coordstart, &lpNumberOfAttrsWritten);

        map[player_pos_new] = TILE_WALL_HORIZ;

        for (int i = 0; i < SCREEN_HEIGHT * SCREEN_WIDTH - 1; ++i){
            screen[i] = ' ';
            if ((map_pos + i >= 0) && 
                (vision[map_pos + i] == 1 
                || level_current->seen[map_pos + i] == 1)) /* keeps walls you've seen on screen*/
                screen[i] = map[map_pos + i];
            if ((map_pos + i) >= SCREEN_HEIGHT * SCREEN_WIDTH - 1)
                screen[i] = ' ';
        }

        if (cursor_pos > -1) // if using the cursor, put it on the screen
            screen[cursor_pos] = 'x';

        // misc stuff to do every iteration
        player_pos_old = player_pos_new;

        while (current_entity){
            current_entity->Pold = current_entity->P;
            current_entity->Xold = current_entity->X % 80;
            current_entity->Yold = current_entity->Y / 80;
            current_entity = current_entity->next;
        }
        current_entity = level_current->entity_liststart;

        if (newturn == 1){
            hud_state(0), hud_update(0);
            //CullAnnouncements();
        }

        if (player.hp <= 0) {
            create_announcement(4, NULL, 0);
            VK = 0;
        }  

        display_announcements();

        for (int i = 0; i < SCREEN_HEIGHT * SCREEN_WIDTH - 1; ++i){
            if (screen_menu[i] != '~')        /* puts menus on screen */
                screen[i] = screen_menu[i];
        }

        // put screen array in console buffer
        WriteConsoleOutputCharacter(hConsole, screen, 
            SCREEN_WIDTH * SCREEN_HEIGHT - 1, coordstart, &dwBytesWritten);

        // hangs if player dies
        while (player.hp <= 0)
            ;

    }
}


