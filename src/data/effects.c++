// src/data/effects.c++ -- the buff / debuff catalogue.
// See docs/FRAMEWORK.md s.6.

#include "spacebattlerpg.h"

// name, category, damage type, stack mode, max stacks, duration, magnitude,
// tick rate, DR category, cleanse, immunity window
static const effectdefinition effectSeeds[] = {
    /* ---- offensive buffs ---- */
    {"Power Surge", EFF_OFFENSIVE, DMG_PHYSICAL, STACK_REFRESH, 1, 10, 20, 0, -1, CLEANSE_NONE, 0},
    {"Berserk", EFF_OFFENSIVE, DMG_PHYSICAL, STACK_REFRESH, 1, 12, 30, 0, -1, CLEANSE_NONE, 0},
    {"Arcane Amplify", EFF_OFFENSIVE, DMG_ARCANE, STACK_REFRESH, 1, 15, 25, 0, -1, CLEANSE_NONE, 0},
    {"Precision", EFF_OFFENSIVE, DMG_PHYSICAL, STACK_ADDITIVE, 3, 20, 5, 0, -1, CLEANSE_NONE, 0},
    {"Elemental Infuse", EFF_OFFENSIVE, DMG_FIRE, STACK_REFRESH, 1, 20, 15, 0, -1, CLEANSE_NONE, 0},

    /* ---- defensive buffs ---- */
    {"Shielded", EFF_DEFENSIVE, DMG_PHYSICAL, STACK_REFRESH, 1, 15, 40, 0, -1, CLEANSE_NONE, 0},
    {"Fortified", EFF_DEFENSIVE, DMG_PHYSICAL, STACK_REFRESH, 1, 12, 25, 0, -1, CLEANSE_NONE, 0},
    {"Regeneration", EFF_DEFENSIVE, DMG_TRUE, STACK_REFRESH, 1, 12, 20, 1, -1, CLEANSE_NONE, 0},
    {"Barrier", EFF_DEFENSIVE, DMG_TRUE, STACK_REFRESH, 1, 10, 60, 0, -1, CLEANSE_NONE, 0},
    {"Anti-Crit", EFF_DEFENSIVE, DMG_PHYSICAL, STACK_REFRESH, 1, 8, 30, 0, -1, CLEANSE_NONE, 0},

    /* ---- utility buffs ---- */
    {"Haste", EFF_UTILITY, DMG_TRUE, STACK_REFRESH, 1, 10, 20, 0, -1, CLEANSE_NONE, 0},
    {"Stealth", EFF_UTILITY, DMG_TRUE, STACK_REFRESH, 1, 8, 1, 0, -1, CLEANSE_NONE, 0},
    {"True Sight", EFF_UTILITY, DMG_TRUE, STACK_REFRESH, 1, 12, 1, 0, -1, CLEANSE_NONE, 0},
    {"Phase Shift", EFF_UTILITY, DMG_TRUE, STACK_REFRESH, 1, 3, 1, 0, -1, CLEANSE_NONE, 2},

    /* ---- crowd control (all share DR category 1) ---- */
    {"Stun", EFF_CROWD_CONTROL, DMG_PHYSICAL, STACK_NONE, 1, 4, 1, 0, 1, CLEANSE_MAGIC, 5},
    {"Freeze", EFF_CROWD_CONTROL, DMG_ICE, STACK_NONE, 1, 5, 1, 0, 1, CLEANSE_MAGIC, 5},
    {"Root", EFF_CROWD_CONTROL, DMG_EARTH, STACK_NONE, 1, 6, 1, 0, 1, CLEANSE_PHYSICAL, 5},
    {"Silence", EFF_CROWD_CONTROL, DMG_ARCANE, STACK_NONE, 1, 5, 1, 0, 1, CLEANSE_MAGIC, 5},
    {"Fear", EFF_CROWD_CONTROL, DMG_DARK, STACK_NONE, 1, 4, 1, 0, 1, CLEANSE_MAGIC, 5},
    {"Sleep", EFF_CROWD_CONTROL, DMG_ARCANE, STACK_NONE, 1, 8, 1, 0, 1, CLEANSE_MAGIC, 5},
    {"Knockdown", EFF_CROWD_CONTROL, DMG_PHYSICAL, STACK_NONE, 1, 3, 1, 0, 1, CLEANSE_PHYSICAL, 5},

    /* ---- damage over time (no DR) ---- */
    {"Burn", EFF_DOT, DMG_FIRE, STACK_ADDITIVE, 5, 12, 30, 1, -1, CLEANSE_MAGIC, 0},
    {"Poison", EFF_DOT, DMG_POISON, STACK_ADDITIVE, 5, 18, 20, 1, -1, CLEANSE_POISON, 0},
    {"Bleed", EFF_DOT, DMG_BLEED, STACK_ADDITIVE, 5, 10, 25, 1, -1, CLEANSE_BLEED, 0},
    {"Corruption", EFF_DOT, DMG_VOID, STACK_REFRESH, 1, 15, 45, 1, -1, CLEANSE_CURSE, 0},
    {"Shock", EFF_DOT, DMG_LIGHTNING, STACK_REFRESH, 1, 8, 35, 1, -1, CLEANSE_MAGIC, 0},
    {"Frostbite", EFF_DOT, DMG_ICE, STACK_REFRESH, 1, 10, 28, 1, -1, CLEANSE_MAGIC, 0},

    /* ---- stat breaks ---- */
    {"Weaken", EFF_STAT_BREAK, DMG_PHYSICAL, STACK_ADDITIVE, 3, 15, -15, 0, -1, CLEANSE_PHYSICAL, 0},
    {"Shatter", EFF_STAT_BREAK, DMG_PHYSICAL, STACK_ADDITIVE, 3, 15, -20, 0, -1, CLEANSE_PHYSICAL, 0},
    {"Curse", EFF_STAT_BREAK, DMG_DARK, STACK_REFRESH, 1, 20, -25, 0, -1, CLEANSE_CURSE, 0},
    {"Slow", EFF_STAT_BREAK, DMG_ICE, STACK_REPLACE_STRONGER, 1, 10, -30, 0, -1, CLEANSE_MAGIC, 0},
    {"Mana Burn", EFF_STAT_BREAK, DMG_ARCANE, STACK_ADDITIVE, 3, 6, -40, 1, -1, CLEANSE_MAGIC, 0}};

static const int effectSeedCount =
    (int)(sizeof(effectSeeds) / sizeof(effectSeeds[0]));

void initializeEffects()
{
    effectCount = 0;

    for (int i = 0; i < effectSeedCount && i < MAX_EFFECTS; ++i)
    {
        effectArray[effectCount] = effectSeeds[i];
        effectCount++;
    }
}

int findEffect(const char *name)
{
    if (!name)
        return -1;

    for (int i = 0; i < effectCount; ++i)
    {
        if (strcmp(effectArray[i].name, name) == 0)
            return i;
    }

    return -1;
}

int countEffectsInCategory(effectcategory category)
{
    int count = 0;

    for (int i = 0; i < effectCount; ++i)
    {
        if (effectArray[i].category == category)
            count++;
    }

    return count;
}

int listEffectsInCategory(effectcategory category, int out[], int outMax)
{
    if (!out || outMax <= 0)
        return 0;

    int written = 0;

    for (int i = 0; i < effectCount && written < outMax; ++i)
    {
        if (effectArray[i].category == category)
        {
            out[written] = i;
            written++;
        }
    }

    return written;
}
