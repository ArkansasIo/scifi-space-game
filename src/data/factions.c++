// src/data/factions.c++ -- galaxy factions and their starting empires.
// See docs/FRAMEWORK.md s.22.

#include "spacebattlerpg.h"

// name, attributes, resources, reputation, at war
static const faction factionSeeds[] = {
    {"Galactic Federation", {80, 80, 90, 70, 75, 60, 70}, {5000, 4000, 3000, 3500, 2500, 0, 0, 0, 0, 0}, 1500, 0},

    {"Outer Rim Coalition", {60, 55, 70, 50, 80, 55, 65}, {2500, 5000, 3500, 2000, 1200, 0, 0, 0, 0, 0}, 800, 0},

    {"Cybernetic Collective", {85, 95, 60, 30, 55, 90, 80}, {4000, 3000, 2000, 4000, 4500, 50, 0, 500, 0, 0}, 200, 0},

    {"Alien Hive", {70, 40, 30, 10, 60, 45, 95}, {1000, 3500, 2500, 1500, 500, 200, 300, 100, 0, 0}, -2000, 1},

    {"Corporate Syndicate", {65, 60, 100, 75, 70, 80, 45}, {9000, 2000, 1500, 2500, 3000, 0, 0, 0, 0, 0}, 400, 0},

    {"Rogue AI", {90, 100, 40, 20, 50, 100, 30}, {1500, 2500, 1000, 5000, 6000, 300, 200, 800, 300, 0}, -3000, 1},

    {"Divine Order", {100, 85, 70, 95, 90, 80, 90}, {20000, 15000, 12000, 15000, 10000, 2000, 1500, 1500, 800, 300}, -5000, 1},

    {"Precursors", {1, 1, 1, 1, 1, 1, 1}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 5}, 0, 0}};

static const int factionSeedCount =
    (int)(sizeof(factionSeeds) / sizeof(factionSeeds[0]));

static const char *buildingNames[] = {
    "Mining Complex", "Research Lab", "Shipyard", "Trade Hub",
    "Defense Grid", "Orbital Cannon", "Habitat Dome"};

static const char *shipClassNames[] = {
    "Fighter", "Corvette", "Frigate", "Destroyer", "Cruiser",
    "Battleship", "Carrier", "Dreadnought", "Titan", "Flagship"};

static const char *researchCategoryNames[] = {
    "Physics", "Engineering", "Biotech", "Psionics"};

void initializeFactions()
{
    factionCount = 0;

    for (int i = 0; i < factionSeedCount && i < MAX_FACTIONS; ++i)
    {
        factionArray[factionCount] = factionSeeds[i];
        factionCount++;
    }
}

int findFactionByName(const char *name)
{
    if (!name)
        return -1;

    for (int i = 0; i < factionCount; ++i)
    {
        if (strcmp(factionArray[i].name, name) == 0)
            return i;
    }

    return -1;
}

int playerFactionIndex()
{
    // The player starts with the Coalition.
    return findFactionByName("Outer Rim Coalition");
}

int buildingCount()
{
    return (int)(sizeof(buildingNames) / sizeof(buildingNames[0]));
}

const char *buildingName(int index)
{
    if (index < 0 || index >= buildingCount())
        return "";

    return buildingNames[index];
}

int shipClassNameCount()
{
    return (int)(sizeof(shipClassNames) / sizeof(shipClassNames[0]));
}

const char *shipClassName(int index)
{
    if (index < 0 || index >= shipClassNameCount())
        return "";

    return shipClassNames[index];
}

int researchCategoryCount()
{
    return (int)(sizeof(researchCategoryNames) / sizeof(researchCategoryNames[0]));
}

const char *researchCategoryName(int index)
{
    if (index < 0 || index >= researchCategoryCount())
        return "";

    return researchCategoryNames[index];
}
