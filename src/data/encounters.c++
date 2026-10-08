// src/data/encounters.c++ -- ready-to-use encounter templates.
// See docs/FRAMEWORK.md s.13.

#include "spacebattlerpg.h"

// name, kind, trash, casters, minibosses, waves, hazard, puzzle, timed, boss
static const encountertemplate encounterSeeds[] = {    /* ---- open world ---- */
    {"Standard Patrol", ENC_PACK, 3, 1, 0, 1, 0, 0, 0, -1},
    {"Heavy Patrol", ENC_PACK, 4, 2, 0, 1, 0, 0, 0, -1},
    {"Rare Spawn: Ash Tyrant", ENC_RARE_SPAWN, 0, 0, 0, 1, 1, 0, 0, 0},
    {"Event Wave", ENC_EVENT_WAVE, 3, 1, 0, 8, 0, 0, 1, 2},

    /* ---- dungeon rooms ---- */
    {"Hazard Cache", ENC_DUNGEON_ROOM, 4, 1, 0, 1, 1, 0, 0, -1},
    {"Miniboss Room", ENC_DUNGEON_ROOM, 3, 1, 1, 1, 0, 0, 0, 1},
    {"Puzzle Chamber", ENC_DUNGEON_ROOM, 2, 2, 0, 1, 0, 1, 0, -1},
    {"Timed Gauntlet", ENC_GAUNTLET, 4, 2, 0, 5, 1, 0, 1, -1},

    /* ---- raid ---- */
    {"Boss and Adds", ENC_RAID_BOSS, 4, 2, 0, 3, 1, 0, 0, 3},
    {"Arena Morph", ENC_RAID_BOSS, 3, 3, 0, 2, 1, 0, 0, 4},
    {"Split Tanks", ENC_COUNCIL, 2, 2, 2, 1, 0, 0, 0, 3},
    {"Council Fight", ENC_COUNCIL, 3, 3, 2, 1, 1, 0, 0, 4}};

static const int encounterSeedCount =
    (int)(sizeof(encounterSeeds) / sizeof(encounterSeeds[0]));

static const char *hazardNames[] = {
    "Toxic Pools", "Fire Trails", "Lightning Orbs",
    "Rotating Lasers", "Falling Meteors", "Gravity Well"};

void initializeEncounters()
{
    encounterCount = 0;

    for (int i = 0; i < encounterSeedCount && i < MAX_ENCOUNTERS; ++i)
    {
        encounterArray[encounterCount] = encounterSeeds[i];
        encounterCount++;
    }
}

int findEncounter(const char *name)
{
    if (!name)
        return -1;

    for (int i = 0; i < encounterCount; ++i)
    {
        if (strcmp(encounterArray[i].name, name) == 0)
            return i;
    }

    return -1;
}

int randomEncounterOfKind(encounterkind kind)
{
    int matches[MAX_ENCOUNTERS];
    int count = 0;

    for (int i = 0; i < encounterCount; ++i)
    {
        if (encounterArray[i].kind == kind)
        {
            matches[count] = i;
            count++;
        }
    }

    if (count <= 0)
        return -1;

    return matches[randomNumber(count) - 1];
}

int hazardCount()
{
    return (int)(sizeof(hazardNames) / sizeof(hazardNames[0]));
}

const char *hazardName(int index)
{
    if (index < 0 || index >= hazardCount())
        return "";

    return hazardNames[index];
}
