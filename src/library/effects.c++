// src/library/effects.c++ -- buffs, debuffs, stacking and diminishing returns.
// See docs/FRAMEWORK.md section 6.

#include "spacebattlerpg.h"

void clearActiveEffect(activeeffect &effect)
{
    effect.definitionIndex = -1;
    effect.stacks = 0;
    effect.remaining = 0;
    effect.magnitude = 0;
    effect.sourceId = -1;
}

int applyEffect(activeeffect list[], int listMax, int definitionIndex,
                int sourceId)
{
    if (!list || listMax <= 0)
        return -1;

    if (definitionIndex < 0 || definitionIndex >= effectCount)
        return -1;

    const effectdefinition &def = effectArray[definitionIndex];

    // Look for an existing instance of the same definition.
    int existing = -1;
    for (int i = 0; i < listMax; ++i)
    {
        if (list[i].definitionIndex == definitionIndex)
        {
            existing = i;
            break;
        }
    }

    switch (def.stackMode)
    {
    case STACK_NONE:
        // Already active: do nothing at all.
        if (existing >= 0)
            return existing;
        break;

    case STACK_REFRESH:
        if (existing >= 0)
        {
            list[existing].remaining = def.duration;
            list[existing].magnitude = def.magnitude;
            return existing;
        }
        break;

    case STACK_ADDITIVE:
        if (existing >= 0)
        {
            if (list[existing].stacks < def.maxStacks)
            {
                list[existing].stacks++;
                list[existing].magnitude += def.magnitude;
            }
            list[existing].remaining = def.duration;
            return existing;
        }
        break;

    case STACK_REPLACE_STRONGER:
        if (existing >= 0)
        {
            if (def.magnitude >= list[existing].magnitude)
            {
                list[existing].magnitude = def.magnitude;
                list[existing].remaining = def.duration;
            }
            return existing;
        }
        break;

    case STACK_INDEPENDENT:
        // Always wants a fresh slot, so fall through to the free-slot scan.
        existing = -1;
        break;

    default:
        break;
    }

    // Allocate the first free slot.
    for (int i = 0; i < listMax; ++i)
    {
        if (list[i].definitionIndex < 0)
        {
            list[i].definitionIndex = definitionIndex;
            list[i].stacks = 1;
            list[i].remaining = def.duration;
            list[i].magnitude = def.magnitude;
            list[i].sourceId = sourceId;
            return i;
        }
    }

    return -1; // no room
}

int tickEffects(activeeffect list[], int listMax)
{
    if (!list || listMax <= 0)
        return 0;

    int expired = 0;

    for (int i = 0; i < listMax; ++i)
    {
        if (list[i].definitionIndex < 0)
            continue;

        if (list[i].remaining > 0)
            list[i].remaining--;

        if (list[i].remaining <= 0)
        {
            clearActiveEffect(list[i]);
            expired++;
        }
    }

    return expired;
}

int diminishingReturnMultiplier(int drCount)
{
    // The CC ladder: 100% -> 50% -> 25% -> 0%.
    switch (drCount)
    {
    case 0:
        return 100;
    case 1:
        return 50;
    case 2:
        return 25;
    default:
        return 0;
    }
}

int applyDiminishingReturns(int baseDuration, int drCount)
{
    int multiplier = diminishingReturnMultiplier(drCount);
    return (baseDuration * multiplier) / 100;
}

bool cleanseMatches(cleansetype cleanse, const effectdefinition &def)
{
    if (cleanse == CLEANSE_NONE)
        return false;

    if (cleanse == CLEANSE_ALL)
        return true;

    return cleanse == def.cleanse;
}

int cleanseEffects(activeeffect list[], int listMax, cleansetype cleanse)
{
    if (!list || listMax <= 0)
        return 0;

    int removed = 0;

    for (int i = 0; i < listMax; ++i)
    {
        int index = list[i].definitionIndex;
        if (index < 0 || index >= effectCount)
            continue;

        if (cleanseMatches(cleanse, effectArray[index]))
        {
            clearActiveEffect(list[i]);
            removed++;
        }
    }

    return removed;
}

int totalMagnitude(const activeeffect list[], int listMax, int definitionIndex)
{
    if (!list || listMax <= 0)
        return 0;

    int total = 0;

    for (int i = 0; i < listMax; ++i)
    {
        if (list[i].definitionIndex == definitionIndex)
            total += list[i].magnitude;
    }

    return total;
}
