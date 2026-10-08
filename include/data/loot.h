// include/data/loot.h -- loot tables for PvE and PvP rewards.
// See docs/FRAMEWORK.md section 12.

#ifndef SBW_DATA_LOOT_H
#define SBW_DATA_LOOT_H

// Fill lootArray[] with drop tables (open world, dungeon, raid, PvP).
void initializeLootTables();

// Look up a loot table by name.  Returns index, or -1.
int findLootTable(const char *name);

// The loot table a given boss drops.  Returns index, or -1.
int lootTableForBossIndex(int bossIndex);

// The loot table used for PvP reward chests.
int pvpLootTableIndex();

// Rarity weights used by a table, written into `out`.
int lootRarityWeights(int tableIndex, int out[]);

#endif /* SBW_DATA_LOOT_H */
