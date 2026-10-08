// src/game/threat.c++ -- the aggro / threat model.
// See docs/FRAMEWORK.md s.9.

#include "spacebattlerpg.h"

void clearThreatTable(threatentry table[], int count)
{
    if (!table || count <= 0)
        return;

    for (int i = 0; i < count; ++i)
    {
        table[i].entityId = -1;
        table[i].threat = 0;
        table[i].forcedUntil = 0;
    }
}

int findThreatIndex(const threatentry table[], int count, int entityId)
{
    if (!table || count <= 0)
        return -1;

    for (int i = 0; i < count; ++i)
    {
        if (table[i].entityId == entityId)
            return i;
    }

    return -1;
}

int computeThreat(int damage, int healing, int flatThreat, int dmgWeight,
                  int healWeight)
{
    // Threat = Damage*D + Healing*H + FlatThreat
    // Healing is weighted below damage so healers do not automatically top
    // the table out of a fight.
    int threat = (damage * dmgWeight) / 100;
    threat += (healing * healWeight) / 100;
    threat += flatThreat;

    return threat;
}

int addThreat(threatentry table[], int count, int entityId, int amount,
              int stancePercent)
{
    if (!table || count <= 0)
        return 0;

    if (stancePercent < 0)
        stancePercent = 0;

    int scaled = (amount * stancePercent) / 100;

    int index = findThreatIndex(table, count, entityId);

    if (index < 0)
    {
        // Claim the first free slot.
        for (int i = 0; i < count; ++i)
        {
            if (table[i].entityId < 0)
            {
                index = i;
                break;
            }
        }
    }

    if (index < 0)
        return 0;

    table[index].entityId = entityId;
    table[index].threat += scaled;

    return table[index].threat;
}

int tauntThreat(threatentry table[], int count, int entityId, int ticks)
{
    if (!table || count <= 0)
        return -1;

    int index = findThreatIndex(table, count, entityId);

    if (index < 0)
    {
        for (int i = 0; i < count; ++i)
        {
            if (table[i].entityId < 0)
            {
                index = i;
                break;
            }
        }
    }

    if (index < 0)
        return -1;

    // Find the current top so the taunt can set threat to top + 1.
    int top = 0;

    for (int i = 0; i < count; ++i)
    {
        if (table[i].entityId >= 0 && table[i].threat > top)
            top = table[i].threat;
    }

    table[index].entityId = entityId;
    table[index].threat = top + 1;
    table[index].forcedUntil = ticks;

    return index;
}

int highestThreatIndex(const threatentry table[], int count)
{
    if (!table || count <= 0)
        return -1;

    int best = -1;
    int bestThreat = -1;

    for (int i = 0; i < count; ++i)
    {
        if (table[i].entityId < 0)
            continue;

        if (table[i].threat > bestThreat)
        {
            bestThreat = table[i].threat;
            best = i;
        }
    }

    return best;
}

int pickTarget(const threatentry table[], int count)
{
    if (!table || count <= 0)
        return -1;

    // A forced target (taunt) wins outright.
    for (int i = 0; i < count; ++i)
    {
        if (table[i].entityId >= 0 && table[i].forcedUntil > 0)
            return table[i].entityId;
    }

    int index = highestThreatIndex(table, count);

    return (index < 0) ? -1 : table[index].entityId;
}

void decayThreat(threatentry table[], int count, int percent)
{
    if (!table || count <= 0)
        return;

    if (percent <= 0)
        return;

    if (percent > 100)
        percent = 100;

    for (int i = 0; i < count; ++i)
    {
        if (table[i].entityId < 0)
            continue;

        table[i].threat -= (table[i].threat * percent) / 100;

        if (table[i].threat < 0)
            table[i].threat = 0;
    }
}

void tickThreat(threatentry table[], int count)
{
    if (!table || count <= 0)
        return;

    for (int i = 0; i < count; ++i)
    {
        if (table[i].forcedUntil > 0)
            table[i].forcedUntil--;
    }
}
