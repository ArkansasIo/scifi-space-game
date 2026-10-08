// src/data/stats.c++ -- stat weights and caps.
// See docs/FRAMEWORK.md s.16.1 (weights) and s.19 (caps).

#include "spacebattlerpg.h"

// A weight row: name -> PvE/PvP value per point.
struct statweight
{
    const char *name;
    int weight;
};

static const statweight statWeights[] = {
    {"STR", 100}, {"DEX", 100}, {"CON", 85}, {"INT", 100}, {"WIS", 85}, {"VIT", 85}, {"SPI", 85}, {"LCK", 90}, {"WIL", 90}, {"CHA", 70}, {"Crit Chance", 250}, {"Crit Damage", 175}, {"Attack Speed", 200}, {"Cooldown Reduction", 225}, {"Armor", 75}, {"Resistance", 90}, {"PvP Resilience", 300}, {"Tenacity", 200}, {"Physical Power", 100}, {"Spell Power", 100}, {"Healing Power", 110}, {"Accuracy", 80}};

static const int statWeightCount =
    (int)(sizeof(statWeights) / sizeof(statWeights[0]));

void initializeStatWeights()
{
    // The weights are const data; this exists so the boot sequence and the
    // loading screen have a symmetric set of initializers to call.
}

int statWeightByName(const char *name)
{
    if (!name)
        return 0;

    for (int i = 0; i < statWeightCount; ++i)
    {
        if (strcmp(statWeights[i].name, name) == 0)
            return statWeights[i].weight;
    }

    return 0;
}

void initializeStatCaps()
{
    statCapCount = 0;

    // name, PvE cap, PvP cap, soft cap, DR constant K
    static const statcap seeds[] = {
        {"Crit Chance", 7000, 4000, 3500, 40},
        {"Crit Damage", 30000, 6000, 20000, 100},
        {"Cooldown Reduction", 6000, 3500, 4000, 50},
        {"Attack Speed", 25000, 15000, 15000, 80},
        {"Damage Reduction", 7000, 7000, 5000, 60},
        {"Healing Bonus", 10000, 5000, 6000, 70},
        {"CC Duration", 10000, 5000, 5000, 55},
        {"Tenacity", 8000, 8000, 4000, 45},
        {"Armor Penetration", 6000, 4000, 3000, 40},
        {"Magic Penetration", 6000, 4000, 3000, 40},
        {"Accuracy", 9500, 9500, 6000, 65},
        {"Evasion", 7500, 5000, 4000, 50},
        {"Resistance", 7500, 7500, 5000, 60},
        {"Movement Speed", 20000, 12000, 10000, 90}};

    int count = (int)(sizeof(seeds) / sizeof(seeds[0]));

    for (int i = 0; i < count && i < MAX_CAPS; ++i)
    {
        statCapArray[statCapCount] = seeds[i];
        statCapCount++;
    }
}

int statCapFor(const char *name, combatcontext context)
{
    if (!name)
        return -1;

    for (int i = 0; i < statCapCount; ++i)
    {
        if (strcmp(statCapArray[i].name, name) != 0)
            continue;

        return (context == CTX_PVP) ? statCapArray[i].pvpCap
                                    : statCapArray[i].pveCap;
    }

    return -1;
}

int statSoftCapFor(const char *name)
{
    if (!name)
        return 0;

    for (int i = 0; i < statCapCount; ++i)
    {
        if (strcmp(statCapArray[i].name, name) == 0)
            return statCapArray[i].softCap;
    }

    return 0;
}
