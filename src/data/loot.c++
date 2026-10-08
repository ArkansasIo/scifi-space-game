// src/data/loot.c++ -- loot tables for PvE and PvP rewards.
// See docs/FRAMEWORK.md s.12.

#include "spacebattlerpg.h"

// name, entry count, entries, pity, unique/week, difficulty mult, distribution
static const loottable lootSeeds[] = {    {"Open World Trash", 3, {{"Scrap", RARITY_COMMON, 60, 1}, {"Alloy", RARITY_UNCOMMON, 30, 1}, {"Circuit", RARITY_RARE, 10, 5}}, 10, 0, 100, LOOT_PERSONAL},

    {"Veteran Cache", 4,
     {{"Scrap", RARITY_COMMON, 40, 5},
      {"Alloy", RARITY_UNCOMMON, 35, 5},
      {"Circuit", RARITY_RARE, 20, 10},
      {"Arc Core", RARITY_EPIC, 5, 15}},
     15, 0, 110, LOOT_NEED_GREED},

    {"Dungeon Boss", 5, {{"Alloy", RARITY_UNCOMMON, 25, 15}, {"Circuit", RARITY_RARE, 35, 20}, {"Arc Core", RARITY_EPIC, 30, 25}, {"Warmind Relic", RARITY_LEGENDARY, 9, 35}, {"Divine Shard", RARITY_MYTHIC, 1, 50}}, 20, 1, 130, LOOT_NEED_GREED},

    {"Raid Boss", 5, {{"Circuit", RARITY_RARE, 15, 30}, {"Arc Core", RARITY_EPIC, 35, 40}, {"Warmind Relic", RARITY_LEGENDARY, 40, 55}, {"Divine Shard", RARITY_MYTHIC, 9, 70}, {"Ascendant Core", RARITY_MYTHIC, 1, 90}}, 25, 1, 160, LOOT_MASTER},

    {"PvP Reward Chest", 3, {{"PvP Token", RARITY_UNCOMMON, 50, 10}, {"Arena Mark", RARITY_RARE, 35, 20}, {"Champion Seal", RARITY_EPIC, 15, 30}}, 12, 3, 120, LOOT_PERSONAL},

    {"World Event", 4, {{"Alloy", RARITY_UNCOMMON, 30, 10}, {"Circuit", RARITY_RARE, 35, 15}, {"Arc Core", RARITY_EPIC, 25, 25}, {"Warmind Relic", RARITY_LEGENDARY, 10, 40}}, 18, 1, 140, LOOT_ROUND_ROBIN}};

static const int lootSeedCount = (int)(sizeof(lootSeeds) / sizeof(lootSeeds[0]));

void initializeLootTables()
{
    lootCount = 0;

    for (int i = 0; i < lootSeedCount && i < MAX_LOOT_TABLES; ++i)
    {
        lootArray[lootCount] = lootSeeds[i];
        lootCount++;
    }
}

int findLootTable(const char *name)
{
    if (!name)
        return -1;

    for (int i = 0; i < lootCount; ++i)
    {
        if (strcmp(lootArray[i].name, name) == 0)
            return i;
    }

    return -1;
}

int lootTableForBossIndex(int bossIndex)
{
    if (bossIndex < 0 || bossIndex >= bossCount)
        return -1;

    // Raid and mythic bosses use the raid table; everything else the dungeon
    // table.  This mirrors the tier ladder rather than per-boss duplication.
    enemytier tier = bossArray[bossIndex].tier;

    if (tier >= TIER_RAID_BOSS)
        return findLootTable("Raid Boss");

    if (tier == TIER_WORLD_BOSS)
        return findLootTable("World Event");

    return findLootTable("Dungeon Boss");
}

int pvpLootTableIndex()
{
    return findLootTable("PvP Reward Chest");
}

int lootRarityWeights(int tableIndex, int out[])
{
    if (tableIndex < 0 || tableIndex >= lootCount)
        return 0;

    if (!out)
        return 0;

    for (int i = 0; i < MAX_RARITIES; ++i)
        out[i] = 0;

    const loottable &table = lootArray[tableIndex];

    for (int i = 0; i < table.entryCount && i < 12; ++i)
    {
        int rarity = (int)table.entries[i].rarity;

        if (rarity >= 0 && rarity < MAX_RARITIES)
            out[rarity] += table.entries[i].weight;
    }

    return 1;
}
