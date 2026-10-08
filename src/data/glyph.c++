// glyph.c++ -- fills glyphsArray[] with the ship glyph list.

#include "spacebattlerpg.h"

struct glyphSeed
{
    const char *name;
    const char *type;
    int bonus;
};

static const glyphSeed glyphSeeds[17] = {
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

void initializeglyphs()
{
    for (int i = 0; i < 17; ++i)
    {
        ship_item &g = glyphsArray[i];

        strncpy(g.name, glyphSeeds[i].name, sizeof(g.name) - 1);
        g.name[sizeof(g.name) - 1] = '\0';

        strncpy(g.type, glyphSeeds[i].type, sizeof(g.type) - 1);
        g.type[sizeof(g.type) - 1] = '\0';

        strncpy(g.itemclass, "Glyph", sizeof(g.itemclass) - 1);
        g.itemclass[sizeof(g.itemclass) - 1] = '\0';

        snprintf(g.description, sizeof(g.description),
                 "A glowing glyph: %s.", glyphSeeds[i].name);

        g.bonus = glyphSeeds[i].bonus;
        g.level = 1;
        g.lightlevel = 1;
        g.attackpower = (strcmp(glyphSeeds[i].type, "Weapon") == 0) ? glyphSeeds[i].bonus * 2 : 0;
        g.defencepower = (strcmp(glyphSeeds[i].type, "Weapon") != 0) ? glyphSeeds[i].bonus * 2 : 0;
    }
}
