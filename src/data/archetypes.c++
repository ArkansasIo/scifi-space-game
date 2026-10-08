// src/data/archetypes.c++ -- enemy archetypes (AI roles).
// See docs/FRAMEWORK.md s.7.1 and s.7.2.

#include "spacebattlerpg.h"

// name, tier, role, preferred range, aggro radius, abilities, loot, weight
static const enemyarchetype archetypeSeeds[] = {
    /* Trash */
    {"Scavenger Drone", TIER_TRASH, ROLE_BRUISER, 1, 3, 1, 0, 20},
    {"Rust Fighter", TIER_TRASH, ROLE_SNIPER, 4, 4, 1, 0, 18},
    {"Mining Sentry", TIER_TRASH, ROLE_CASTER, 3, 3, 2, 0, 14},

    /* Veteran */
    {"Veteran Raider", TIER_VETERAN, ROLE_BRUISER, 1, 5, 3, 1, 14},
    {"Veteran Marksman", TIER_VETERAN, ROLE_SNIPER, 6, 6, 3, 1, 12},
    {"Veteran Adept", TIER_VETERAN, ROLE_CASTER, 5, 5, 4, 1, 10},

    /* Elite */
    {"Elite Vanguard", TIER_ELITE, ROLE_TANK, 1, 6, 5, 2, 8},
    {"Elite Hunter", TIER_ELITE, ROLE_ASSASSIN, 2, 8, 5, 2, 7},
    {"Elite Warden", TIER_ELITE, ROLE_CONTROLLER, 4, 6, 5, 2, 6},
    {"Elite Summoner", TIER_ELITE, ROLE_SUMMONER, 7, 6, 5, 2, 5},
    {"Elite Medic", TIER_ELITE, ROLE_SUPPORT, 5, 5, 5, 2, 4},

    /* Champion */
    {"Champion Breaker", TIER_CHAMPION, ROLE_BRUISER, 1, 8, 6, 3, 4},
    {"Champion Shade", TIER_CHAMPION, ROLE_TRICKSTER, 3, 9, 6, 3, 3},
    {"Champion Oracle", TIER_CHAMPION, ROLE_CONTROLLER, 6, 8, 6, 3, 3},

    /* Dungeon boss */
    {"Dungeon Tyrant", TIER_DUNGEON_BOSS, ROLE_BRUISER, 1, 10, 8, 4, 2},
    {"Dungeon Archon", TIER_DUNGEON_BOSS, ROLE_CASTER, 5, 10, 8, 4, 2},

    /* World boss */
    {"World Devourer", TIER_WORLD_BOSS, ROLE_BRUISER, 2, 14, 10, 5, 1},
    {"World Harbinger", TIER_WORLD_BOSS, ROLE_SUMMONER, 8, 14, 10, 5, 1},

    /* Raid boss */
    {"Raid Sovereign", TIER_RAID_BOSS, ROLE_TANK, 1, 16, 12, 6, 1},
    {"Raid Mind", TIER_RAID_BOSS, ROLE_CONTROLLER, 6, 16, 12, 6, 1},
    {"Raid Trickster", TIER_RAID_BOSS, ROLE_TRICKSTER, 4, 16, 12, 6, 1},

    /* Mythic */
    {"Mythic Ascendant", TIER_MYTHIC, ROLE_BRUISER, 1, 20, 14, 7, 1},
    {"Mythic Void", TIER_MYTHIC, ROLE_CASTER, 6, 20, 14, 7, 1},
    {"Mythic Phantom", TIER_MYTHIC, ROLE_ASSASSIN, 2, 20, 14, 7, 1}};

static const int archetypeSeedCount =
    (int)(sizeof(archetypeSeeds) / sizeof(archetypeSeeds[0]));

void initializeArchetypes()
{
    archetypeCount = 0;

    for (int i = 0; i < archetypeSeedCount && i < MAX_ARCHETYPES; ++i)
    {
        archetypeArray[archetypeCount] = archetypeSeeds[i];
        archetypeCount++;
    }
}

int findArchetype(enemyrole role, enemytier tier)
{
    for (int i = 0; i < archetypeCount; ++i)
    {
        if (archetypeArray[i].role == role && archetypeArray[i].tier == tier)
            return i;
    }

    return -1;
}

int randomArchetypeForTier(enemytier tier)
{
    int matches[MAX_ARCHETYPES];
    int count = 0;

    for (int i = 0; i < archetypeCount; ++i)
    {
        if (archetypeArray[i].tier == tier)
        {
            matches[count] = i;
            count++;
        }
    }

    if (count <= 0)
        return -1;

    return matches[randomNumber(count) - 1];
}

int archetypeHPMultiplier(int archetypeIndex)
{
    if (archetypeIndex < 0 || archetypeIndex >= archetypeCount)
        return 100;

    // HP uses the tier baseline directly.
    return tierBaselineFor(archetypeArray[archetypeIndex].tier);
}

int archetypeDamageMultiplier(int archetypeIndex)
{
    if (archetypeIndex < 0 || archetypeIndex >= archetypeCount)
        return 100;

    // Damage scales from the same baseline, deliberately damped so it always
    // climbs slower than HP (docs/FRAMEWORK.md s.21).
    int baseline = tierBaselineFor(archetypeArray[archetypeIndex].tier);

    return 100 + ((baseline - 100) * 6) / 10;
}
