// artifact.c++ -- fills artifactArray[] with the ship artifact list.

#include "spacebattlerpg.h"

struct artifactSeed
{
    const char *name;
    const char *type;
    int bonus;
};

static const artifactSeed artifactSeeds[17] = {
    {"Photon Torpedoes", "Weapon", 20},
    {"Ion Cannons", "Weapon", 30},
    {"Forward Disruptors", "Weapon", 40},
    {"Borg Shields", "Shield", 20},
    {"Ori Shields", "Shield", 30},
    {"Cloaking", "Shield", 40},
    {"Ablative Armour Type 1", "Armour", 10},
    {"Ablative Armour Type 2", "Armour", 20},
    {"Drone Weapon", "Weapon", 20},
    {"Chroniton Torpedoes", "Weapon", 30},
    {"Tricobalt Device", "Weapon", 40},
    {"Isolytic Burst", "Weapon", 40},
    {"Pinpoint Barrier", "Shield", 10},
    {"Point Defense Lasers", "Shield", 20},
    {"Omni-Directional Barrier", "Shield", 30},
    {"Particle Weapon", "Weapon", 20},
    {"Transphasic Torpedo", "Weapon", 50}};

void initializeartifact()
{
    for (int i = 0; i < 17; ++i)
    {
        ship_item &a = artifactArray[i];

        strncpy(a.name, artifactSeeds[i].name, sizeof(a.name) - 1);
        a.name[sizeof(a.name) - 1] = '\0';

        strncpy(a.type, artifactSeeds[i].type, sizeof(a.type) - 1);
        a.type[sizeof(a.type) - 1] = '\0';

        strncpy(a.itemclass, "Artifact", sizeof(a.itemclass) - 1);
        a.itemclass[sizeof(a.itemclass) - 1] = '\0';

        snprintf(a.description, sizeof(a.description),
                 "An ancient artifact: %s.", artifactSeeds[i].name);

        a.bonus = artifactSeeds[i].bonus;
        a.level = 1;
        a.lightlevel = 1;
        a.attackpower = (strcmp(artifactSeeds[i].type, "Weapon") == 0) ? artifactSeeds[i].bonus * 2 : 0;
        a.defencepower = (strcmp(artifactSeeds[i].type, "Weapon") != 0) ? artifactSeeds[i].bonus * 2 : 0;
    }
}
