// src/data/affixes.c++ -- elite / champion / mythic affixes.
// See docs/FRAMEWORK.md s.11.

#include "spacebattlerpg.h"

// name, category, magnitude, stack cost
static const affix affixSeeds[] = {
    /* Offensive */
    {"Berserk", AFFIX_OFFENSIVE, 35, 2},
    {"Frenzied", AFFIX_OFFENSIVE, 50, 3},
    {"Sniper", AFFIX_OFFENSIVE, 25, 1},
    {"Arcane Burst", AFFIX_OFFENSIVE, 40, 2},
    {"Vampiric", AFFIX_OFFENSIVE, 15, 2},

    /* Defensive */
    {"Fortified", AFFIX_DEFENSIVE, 40, 2},
    {"Shielded", AFFIX_DEFENSIVE, 60, 2},
    {"Reflective", AFFIX_DEFENSIVE, 20, 3},

    /* Utility */
    {"Teleporting", AFFIX_UTILITY, 1, 2},
    {"Summoner", AFFIX_UTILITY, 1, 2},
    {"Illusionist", AFFIX_UTILITY, 1, 1},

    /* Hazard */
    {"Toxic Pools", AFFIX_HAZARD, 30, 2},
    {"Fire Trails", AFFIX_HAZARD, 35, 2},
    {"Lightning Orbs", AFFIX_HAZARD, 40, 3},

    /* Rules */
    {"No Resurrection", AFFIX_RULE, 0, 1},
    {"Time Pressure", AFFIX_RULE, 0, 1}};

static const int affixSeedCount =
    (int)(sizeof(affixSeeds) / sizeof(affixSeeds[0]));

void initializeAffixes()
{
    affixCount = 0;

    for (int i = 0; i < affixSeedCount && i < MAX_AFFIXES; ++i)
    {
        affixArray[affixCount] = affixSeeds[i];
        affixCount++;
    }
}

int findAffix(const char *name)
{
    if (!name)
        return -1;

    for (int i = 0; i < affixCount; ++i)
    {
        if (strcmp(affixArray[i].name, name) == 0)
            return i;
    }

    return -1;
}

int affixesConflict(int affixA, int affixB)
{
    if (affixA < 0 || affixA >= affixCount)
        return 0;

    if (affixB < 0 || affixB >= affixCount)
        return 0;

    // The unfun combination: a reflective target plus a high damage-over-time
    // aura punishes the player for playing correctly, so it is banned.
    const affix &a = affixArray[affixA];
    const affix &b = affixArray[affixB];

    bool aReflect = (strcmp(a.name, "Reflective") == 0);
    bool bReflect = (strcmp(b.name, "Reflective") == 0);

    bool aDotHazard = (a.category == AFFIX_HAZARD && a.magnitude >= 30);
    bool bDotHazard = (b.category == AFFIX_HAZARD && b.magnitude >= 30);

    if ((aReflect && bDotHazard) || (bReflect && aDotHazard))
        return 1;

    return 0;
}

int rollAffixSet(affixcategory seasonCategory, int out[], int outMax)
{
    if (!out || outMax <= 0)
        return 0;

    int written = 0;

    // Rule: Minor (1-2) + Major (0-1) + Seasonal (0-1).  The bucket budget
    // is enforced with a stack-cost total of 4.
    int budget = 4;    int majorTaken = 0;
    int guard = 0;

    while (written < outMax && budget > 0 && guard < 64)
    {
        guard++;

        int candidate = randomNumber(affixCount) - 1;

        if (candidate < 0 || candidate >= affixCount)
            continue;

        const affix &affix = affixArray[candidate];

        if (affix.stackCost > budget)
            continue;

        // Major slot: only one offensive/defensive/hazard major allowed.
        bool isMajor = (affix.category == AFFIX_OFFENSIVE || affix.category == AFFIX_DEFENSIVE || affix.category == AFFIX_HAZARD);

        if (isMajor && majorTaken >= 1)
            continue;

        // Seasonal slot.
        if (seasonCategory != AFFIX_OFFENSIVE && affix.category == seasonCategory && written >= outMax - 1)
            continue;

        // Refuse duplicates and forbidden pairings.
        bool duplicate = false;
        bool conflict = false;

        for (int i = 0; i < written; ++i)
        {
            if (out[i] == candidate)
                duplicate = true;

            if (affixesConflict(out[i], candidate))
                conflict = true;
        }

        if (duplicate || conflict)
            continue;

        out[written] = candidate;
        written++;
        budget -= affix.stackCost;

        if (isMajor)
            majorTaken++;
    }

    return written;
}
