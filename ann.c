/*
    Everything to do with the announcements at the bottom of the screen. 
*/

#include "game.h"

extern char screen_menu[SCREEN_HEIGHT * SCREEN_WIDTH - 1]; 

announcement* ann_liststart;

void create_announcement(int arg, char* name, int val)
{
    char* str = malloc(sizeof(char)*50);

    switch (arg) {
        case 0 : sprintf(str, "The %s attacks you for %d damage", name, val); break;
        case 1 : sprintf(str, "You attack the %s for %d damage", name, val); break;
        case 2 : sprintf(str, "You pick up a %s", name); break;
        case 3 : sprintf(str, "You kill the %s", name); break;
        case 4 : sprintf(str, "You died"); break;
    };

    announcement* temp = malloc(sizeof(announcement));

    temp->str = str;
    temp->previous = temp->next = NULL;

    if (ann_liststart == NULL){
        ann_liststart = temp;
        return;
    }

    temp->next = ann_liststart;
    ann_liststart->previous = temp;
    ann_liststart = temp;
}

void display_announcements()
{
    char* ann1 = NULL; 
    char* ann2 = NULL; 
    char* ann3 = NULL;

    if (ann_liststart != NULL){
        ann1 = ann_liststart->str;
        if (ann_liststart->next != NULL){
            ann2 = ann_liststart->next->str;
            if (ann_liststart->next->next != NULL)
                ann3 = ann_liststart->next->next->str;
        }
    }

    for (int i = 0; i < 50; i++){       // clears the space on hud array
        screen_menu[80*24 + i] = ' ';
        screen_menu[80*23 + i] = ' ';
        screen_menu[80*22 + i] = ' ';
    }

    if (ann1 != NULL)
        memcpy(screen_menu+80*24, ann1, strlen(ann1));
    if (ann2 != NULL)
        memcpy(screen_menu+80*23, ann2, strlen(ann2));


}

// void CullAnnouncements()
// {
//     announcement* current = ann_listend;

//     while (current != NULL) {
//         current = current->previous;
//         if (current != NULL)
//             free(current->next);
//     }
// }
