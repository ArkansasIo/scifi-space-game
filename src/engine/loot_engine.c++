// src/engine/loot_engine.c++ -- the loot engine.
//
// Rolls drop tables with rarity weights, pity timers and bounded luck, then
// applies reward multipliers from difficulty, boss rank, clear time and the
// no-death bonus.  See docs/FRAMEWORK.md s.12.

#include "spacebattlerpg.h"
#include "engine/loot_engine.h"

static int g_lootRunning = 0;

/* ---------------- lifecycle ---------------- */

int lootEngineInit()
{
    return 1;
}

int lootEngineStart()
{
    g_lootRunning = 1;
    return 1;
}

int lootEngineUpdate(int ticks)
{
    (void)ticks;
    return g_lootRunning;
}

void lootEngineStop()
{
    g_lootRunning = 0;
}

void lootEngineShutdown()
{
    g_lootRunning = 0;
}

/* ---------------- runtime ---------------- */

void clearLootRuntime(lootruntime &rt)
{
    memset(&rt, 0, sizeof(rt));
}

/* ---------------- rolls ---------------- */

// Total weight of every entry in a table.
static int lootTotalWeight(int tableIndex)
{
    if (tableIndex < 0 || tableIndex >= lootCount)
        return 0;

    const loottable &table = lootArray[tableIndex];
    int total = 0;

    for (int i = 0; i < table.entryCount && i < 12; ++i)
        total += table.entries[i].weight;

    return total;
}

gearrarity lootRollRarity(lootruntime &rt, int tableIndex, int luck)
{
    // Pity timer: after enough unlucky rolls, force a rare or better.
    if (tableIndex >= 0 && tableIndex < lootCount)
    {
        const loottable &table = lootArray[tableIndex];

        if (table.pityTimer > 0 && rt.rollsSinceRare >= table.pityTimer)
        {
            rt.rollsSinceRare = 0;
            return RARITY_RARE;
        }
    }

    // Luck biases the roll upward, but only within its bound.
    int roll = randomNumber(1000);
    roll -= lootBoundLuck(luck);

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

int lootRoll(lootruntime &rt, int tableIndex, int difficultyTier,
             int partySize, int luck, lootresult out[], int outMax)
{
    (void)luck;  // luck biases rarity, applied in lootRollRarity

    if (tableIndex < 0 || tableIndex >= lootCount)
        return -1;

    if (!out || outMax <= 0)
        return 0;

    const loottable &table = lootArray[tableIndex];

    int totalWeight = lootTotalWeight(tableIndex);
    if (totalWeight <= 0)
        return 0;

    // One roll per player, plus the table's own base entry count.
    int rolls = table.entryCount + (partySize - 1);
    if (rolls > outMax)
        rolls = outMax;

    if (rolls < 1)
        rolls = 1;

    int written = 0;

    for (int r = 0; r < rolls; ++r)
    {
        int pick = randomNumber(totalWeight) - 1;
        int running = 0;
        int chosen = -1;

        for (int i = 0; i < table.entryCount && i < 12; ++i)
        {
            running += table.entries[i].weight;

            if (pick < running)
            {
                chosen = i;
                break;
            }
        }

        if (chosen < 0 && table.entryCount > 0)
            chosen = table.entryCount - 1;

        if (chosen < 0)
            break;

        out[written].entryIndex = chosen;
        out[written].rarity = table.entries[chosen].rarity;
        out[written].itemLevel = table.entries[chosen].minLevel + difficultyTier * 5;
        out[written].quantity = 1;

        written++;
        rt.totalRolls++;

        if (out[written - 1].rarity >= RARITY_RARE)
        {
            rt.rareDrops++;
            rt.rollsSinceRare = 0;
        }
        else
        {
            rt.rollsSinceRare++;
        }
    }

    // Reward multipliers scale the drop's level, not the drop count, so loot
    // pressure stays manageable.
    int multiplier = lootRewardMultiplier(difficultyTier,
                                          TIER_DUNGEON_BOSS, 100, 0);
    multiplier = multiplier > 0 ? multiplier : 100;

    for (int i = 0; i < written; ++i)
        out[i].itemLevel = (out[i].itemLevel * multiplier) / 100;

    luaFireEvent(SBW_EVENT_LOOT_ROLLED, tableIndex, written);

    return written;
}

void lootUpdatePity(lootruntime &rt, gearrarity rolled, gearrarity target)
{
    if (rolled >= target)
        rt.rollsSinceRare = 0;
    else
        rt.rollsSinceRare++;
}

int lootRewardMultiplier(int difficultyTier, enemytier tier,
                         int clearTimePercent, int noDeath)
{
    // Difficulty tier ramps the reward.
    int multiplier = 100 + (difficultyTier * 20);

    // Boss rank ramps it further, using the shared tier multiplier.
    multiplier = (multiplier * tierMultiplierFor(tier)) / 100;

    // A fast clear is worth more; 100% means "on pace".
    if (clearTimePercent > 0)
        multiplier = (multiplier * 100) / clearTimePercent;

    // Surviving the run untouched is worth a flat bonus.
    if (noDeath)
        multiplier += 15;

    if (multiplier < 10)
        multiplier = 10;

    return multiplier;
}

int lootBoundLuck(int rawLuck)
{
    // Luck is hard-capped so it can never break the economy.
    int cap = 50;

    if (rawLuck > cap)
        return cap;

    if (rawLuck < 0)
        return 0;

    return rawLuck;
}

int lootDistribute(lootdistribution rule, int partySize, int dropIndex)
{
    if (partySize <= 0)
        return 0;

    switch (rule)
    {
    case LOOT_PERSONAL:
        // Every player gets their own copy.
        return 0;

    case LOOT_NEED_GREED:
    case LOOT_MASTER:
        return randomNumber(partySize) - 1;

    case LOOT_ROUND_ROBIN:
        return dropIndex % partySize;

    default:
        return 0;
    }
}

/* ---------------- reporting ---------------- */

void lootShowSummary(const lootruntime &rt)
{
    cout << "  Loot rolls: " << rt.totalRolls
         << "   rare or better: " << rt.rareDrops << endl;

    if (rt.totalRolls > 0)
    {
        int rate = (rt.rareDrops * 100) / rt.totalRolls;
        cout << "  Rare drop rate: " << rate << "%" << endl;
    }

    cout << endl;
}

int lootDropCount(const lootresult out[], int count, gearrarity rarity)
{
    if (!out || count <= 0)
        return 0;

    int found = 0;

    for (int i = 0; i < count; ++i)
    {
        if (out[i].rarity >= rarity)
            found++;
    }

    return found;
}
