// src/game/loot.c++ -- loot rolls, pity timers and reward multipliers.
// See docs/FRAMEWORK.md s.12.

#include "spacebattlerpg.h"

int boundedLuck(int rawLuck, int cap)
{
    if (cap <= 0)
        cap = 50;

    if (rawLuck > cap)
        return cap;

    if (rawLuck < 0)
        return 0;

    return rawLuck;
}

gearrarity rollRarity(const loottable &table, int luck)
{
    // Pity timer first: bad luck protection outranks the raw roll.
    if (table.pityTimer > 0)
    {
        static int lastPity = 0;
        lastPity++;

        if (lastPity >= table.pityTimer)
        {
            lastPity = 0;
            return RARITY_RARE;
        }
    }

    int roll = randomNumber(1000);
    roll -= boundedLuck(luck, 50);

    if (roll < 0)
        roll = 0;

    if (roll <= 5)
        return RARITY_MYTHIC;
    if (roll <= 25)
        return RARITY_LEGENDARY;
    if (roll <= 100)
        return RARITY_EPIC;
    if (roll <= 300)
        return RARITY_RARE;
    if (roll <= 600)
        return RARITY_UNCOMMON;

    return RARITY_COMMON;
}

int rollLoot(const loottable &table, int difficultyTier, int partySize,
             int luck, lootresult out[], int outMax)
{
    // Luck is applied inside rollRarity(); bound it here so a stacked bonus
    // cannot skew the table.
    luck = boundedLuck(luck, 50);
    (void)luck;

    if (!out || outMax <= 0)
        return 0;

    if (table.entryCount <= 0)
        return 0;

    int rolls = table.entryCount + (partySize > 1 ? partySize - 1 : 0);

    if (rolls > outMax)
        rolls = outMax;

    int written = 0;

    for (int r = 0; r < rolls; ++r)
    {
        // Walk the weight list to pick one entry.
        int totalWeight = 0;

        for (int i = 0; i < table.entryCount && i < 12; ++i)
            totalWeight += table.entries[i].weight;

        if (totalWeight <= 0)
            break;

        int pick = randomNumber(totalWeight);
        int running = 0;
        int chosen = -1;

        for (int i = 0; i < table.entryCount && i < 12; ++i)
        {
            running += table.entries[i].weight;

            if (pick <= running)
            {
                chosen = i;
                break;
            }
        }

        if (chosen < 0)
            chosen = table.entryCount - 1;

        out[written].entryIndex = chosen;
        out[written].rarity = table.entries[chosen].rarity;
        out[written].itemLevel = table.entries[chosen].minLevel + difficultyTier * 5;
        out[written].quantity = 1;

        written++;

        luaFireEvent(SBW_EVENT_LOOT_ROLLED, chosen, written);
    }

    return written;
}

int rewardMultiplier(int difficultyTier, enemytier tier,
                     int clearTimePercent, int noDeath)
{
    // Base grows with the difficulty tier.
    int multiplier = 100 + (difficultyTier * 20);

    // Then by the boss rank, using the shared tier multiplier.
    multiplier = (multiplier * tierMultiplierFor(tier)) / 100;

    // A fast clear is worth more; 100% means "on pace".
    if (clearTimePercent > 0)
        multiplier = (multiplier * 100) / clearTimePercent;

    // No-death runs get a flat bonus.
    if (noDeath)
        multiplier += 15;

    if (multiplier < 10)
        multiplier = 10;

    return multiplier;
}

int updatePityTimer(int pityTimer, gearrarity rolled, gearrarity target)
{
    // Returns the new consecutive-bad-roll streak.
    if (rolled >= target)
        return 0;

    return pityTimer + 1;
}
