// technology.c++ -- fills TechnologyArray[] with the researchable tech list.

#include "spacebattlerpg.h"

struct techSeed
{
    const char *name;
    const char *type;
    int bonus;
};

static const techSeed techSeeds[17] = {
    {"Photon Torpedoes", "Weapon", 20},
    {"Ion Cannons", "Weapon", 30},
    {"Forward Disruptors", "Weapon", 40},
    {"Borg Shields", "Shield", 20},
    {"Ori Shields", "Shield", 30},
    {"Cloaking", "Shield", 40},
    {"Ablative Armour Type 1", "Armour", 10},
    {"Ablative Armour Type 2", "Armour", 20},
    {"Drone Weapon", "Weapon", 20},
    {"Chroniton Torpedoes", "Weapon", 40},
    {"Tricobalt Device", "Weapon", 50},
    {"Isolytic Burst", "Weapon", 40},
    {"Pinpoint Barrier", "Shield", 10},
    {"Point Defense Lasers", "Shield", 20},
    {"Omni-Directional Barrier", "Shield", 40},
    {"Particle Weapon", "Weapon", 20},
    {"Transphasic Torpedo", "Weapon", 50}};

void initializeTechnology()
{
    for (int i = 0; i < 17; ++i)
    {
        Technology &t = TechnologyArray[i];

        strncpy(t.name, techSeeds[i].name, sizeof(t.name) - 1);
        t.name[sizeof(t.name) - 1] = '\0';

        strncpy(t.type, techSeeds[i].type, sizeof(t.type) - 1);
        t.type[sizeof(t.type) - 1] = '\0';

        strncpy(t.itemclass, "Standard", sizeof(t.itemclass) - 1);
        t.itemclass[sizeof(t.itemclass) - 1] = '\0';

        t.bonus = techSeeds[i].bonus;
        t.level = 1;
        t.lightlevel = 1;
        t.attackpower = (strcmp(techSeeds[i].type, "Weapon") == 0) ? techSeeds[i].bonus * 2 : 0;
        t.defencepower = (strcmp(techSeeds[i].type, "Weapon") != 0) ? techSeeds[i].bonus * 2 : 0;
    }
}
