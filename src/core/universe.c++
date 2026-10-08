// src/core/universe.c++ -- global game data and the space map.
// This is the file that actually DEFINES the arrays declared "extern" in
// include/core/globals.h.  It must be compiled exactly once.

#include "spacebattlerpg.h"

/* ---------------- global objects ---------------- */

space spaceArray[17];
Technology TechnologyArray[17];
upgrads upgradeArray[17];
ship_item artifactArray[17];
ship_item glyphsArray[17];
ship_item enchantmentArray[17];
player charactership;
enemyships enemyshipsArray[17];

difficultmode difficultMode;
targetscore targetScore;

/* ---------------- space map ---------------- */

// Names of the 17 star systems the player can travel between.
static const char *planetNames[17] = {
    "Aaamazzara", "Altair IV", "Aurelia", "Bajor", "Benthos",
    "Borg Prime", "Cait", "Cardassia Prime", "Cygnia Minor", "Daran V",
    "Duronom", "Dytallix B", "Efros", "El-Adrel IV", "Epsilon Caneris III",
    "Ferenginar", "Finnea Prime"};

// The map is a simple ring: each node links to the next and the previous one.
static void fillSpace(space arr[], int count)
{
    for (int i = 0; i < count; ++i)
    {
        strncpy(arr[i].name, planetNames[i], sizeof(arr[i].name) - 1);
        arr[i].name[sizeof(arr[i].name) - 1] = '\0';

        snprintf(arr[i].description, sizeof(arr[i].description),
                 "You are in the %s system.\n", planetNames[i]);

        arr[i].forward = (i + 1) % count;
        arr[i].backward = (i - 1 + count) % count;
        arr[i].starboard = (i + 1) % count;
        arr[i].port = (i - 1 + count) % count;
    }
}

void initializespace() { fillSpace(spaceArray, 17); }
void initializemspace() { fillSpace(spaceArray, 17); }
void initializeqspace() { fillSpace(spaceArray, 17); }
void initializeuspace() { fillSpace(spaceArray, 17); }
void initializesolspace() { fillSpace(spaceArray, 17); }
