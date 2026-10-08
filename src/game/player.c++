// player.c++ -- creates and initializes the player's ship.

#include "spacebattlerpg.h"

void initializecreatPlayerdata()
{
    memset(&charactership, 0, sizeof(charactership));
    strncpy(charactership.name, "Player Ship", sizeof(charactership.name) - 1);
}

void initializePlayer()
{
    strncpy(charactership.name, "USS Enterprise", sizeof(charactership.name) - 1);
    charactership.name[sizeof(charactership.name) - 1] = '\0';

    charactership.power = 100; // power
    charactership.sif = 50;    // structural integrity field
    charactership.sp = 100;    // ship points
    charactership.exp = 0;
    charactership.health = 100;
    charactership.level = 1;
    charactership.lightlevel = 1;
    charactership.attackpower = 100;
    charactership.defencepower = 100;
    charactership.weapon = 10; // weapon bonus
    charactership.armour = 10; // armour bonus
    charactership.shield = 10; // shield bonus
}