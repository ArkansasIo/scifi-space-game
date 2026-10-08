// upgeadeclass.c -- fills upgradeArray[] with the technology upgrade list.
// (Original file had the extension .c which the C++ compiler would not
//  compile as C++; it is written here as plain C-compatible C++.)

#include "spacebattlerpg.h"

struct upgradeSeed
{
    const char *name;
    const char *type;
    int bonus;
};

static const upgradeSeed upgradeSeeds[17] = {
    {"Photon Torpedoes Mark II", "Weapon", 25},
    {"Ion Cannons Mark II", "Weapon", 35},
    {"Forward Disruptors Mark II", "Weapon", 45},
    {"Borg Shields Mark II", "Shield", 25},
    {"Ori Shields Mark II", "Shield", 35},
    {"Cloaking Mark II", "Shield", 45},
    {"Ablative Armour Type 3", "Armour", 15},
    {"Ablative Armour Type 4", "Armour", 25},
    {"Drone Weapon Mark II", "Weapon", 25},
    {"Chroniton Torpedoes Mark II", "Weapon", 45},
    {"Tricobalt Device Mark II", "Weapon", 55},
    {"Isolytic Burst Mark II", "Weapon", 45},
    {"Pinpoint Barrier Mark II", "Shield", 15},
    {"Point Defense Lasers Mark II", "Shield", 25},
    {"Omni Barrier Mark II", "Shield", 45},
    {"Particle Weapon Mark II", "Weapon", 25},
    {"Transphasic Torpedo Mark II", "Weapon", 55}};

void initializeupgrades()
{
    for (int i = 0; i < 17; ++i)
    {
        upgrads &u = upgradeArray[i];

        strncpy(u.name, upgradeSeeds[i].name, sizeof(u.name) - 1);
        u.name[sizeof(u.name) - 1] = '\0';

        strncpy(u.type, upgradeSeeds[i].type, sizeof(u.type) - 1);
        u.type[sizeof(u.type) - 1] = '\0';

        strncpy(u.itemclass, "Upgrade", sizeof(u.itemclass) - 1);
        u.itemclass[sizeof(u.itemclass) - 1] = '\0';

        u.bonus = upgradeSeeds[i].bonus;
        u.level = 2;
        u.lightlevel = 2;
        u.attackpower = (strcmp(upgradeSeeds[i].type, "Weapon") == 0) ? upgradeSeeds[i].bonus * 2 : 0;
        u.defencepower = (strcmp(upgradeSeeds[i].type, "Weapon") != 0) ? upgradeSeeds[i].bonus * 2 : 0;
    }
}
