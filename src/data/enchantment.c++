// enchantiments.c++ -- fills enchantmentArray[] with the ship enchantment list.

#include "spacebattlerpg.h"

struct enchantmentSeed
{
    const char *name;
    const char *type;
    int bonus;
};

static const enchantmentSeed enchantmentSeeds[17] = {
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

void initializeenchantment()
{
    for (int i = 0; i < 17; ++i)
    {
        ship_item &e = enchantmentArray[i];

        strncpy(e.name, enchantmentSeeds[i].name, sizeof(e.name) - 1);
        e.name[sizeof(e.name) - 1] = '\0';

        strncpy(e.type, enchantmentSeeds[i].type, sizeof(e.type) - 1);
        e.type[sizeof(e.type) - 1] = '\0';

        strncpy(e.itemclass, "Enchantment", sizeof(e.itemclass) - 1);
        e.itemclass[sizeof(e.itemclass) - 1] = '\0';

        snprintf(e.description, sizeof(e.description),
                 "A powerful enchantment: %s.", enchantmentSeeds[i].name);

        e.bonus = enchantmentSeeds[i].bonus;
        e.level = 1;
        e.lightlevel = 1;
        e.attackpower = (strcmp(enchantmentSeeds[i].type, "Weapon") == 0) ? enchantmentSeeds[i].bonus * 2 : 0;
        e.defencepower = (strcmp(enchantmentSeeds[i].type, "Weapon") != 0) ? enchantmentSeeds[i].bonus * 2 : 0;
    }
}
