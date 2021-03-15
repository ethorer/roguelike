#include "game.h"

entity_stats player;
void (*queued_effect)();

// this file will be for spells/effects

void melee_attack(entity* target)
{
    int atk = (player.atk + random(0, 3)) - target->stats->def;
    target->HP -= atk;
    create_announcement(1, target->stats->name, atk);
    if (target->HP <= 0){
        // if (random(0, 10) < 3)
        //     spawn_item(target->P, generate_item(&items[0]));
        create_announcement(3, target->stats->name, 0);
    }
}

void health_restore()
{
    if ((player.hp += 3) > 10)
        player.hp = 10;
}

void health_restore_wrapper()
{
    queued_effect = health_restore;
}
