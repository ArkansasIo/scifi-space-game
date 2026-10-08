// include/game/loot.h -- loot rolls, pity timers and reward multipliers.
// See docs/FRAMEWORK.md section 12.

#ifndef SBW_GAME_LOOT_H
#define SBW_GAME_LOOT_H

// One rolled drop.
struct lootresult
{
    int entryIndex;
    gearrarity rarity;
    int itemLevel;
    int quantity;
};

// Roll against a loot table.  Returns how many entries dropped, filling
// `out` (capacity `outMax`).
int rollLoot(const loottable &table, int difficultyTier, int partySize,
             int luck, lootresult out[], int outMax);

// Pick a rarity using the table's weights, honouring the pity timer.
gearrarity rollRarity(const loottable &table, int luck);

// RewardMultiplier from difficulty, boss rank, clear time and no-death bonus.
int rewardMultiplier(int difficultyTier, enemytier tier, int clearTimePercent,
                     int noDeath);

// Bounded luck: never allowed to break the economy.
int boundedLuck(int rawLuck, int cap);

// Advance a pity timer after a roll.  Returns the updated value.
int updatePityTimer(int pityTimer, gearrarity rolled, gearrarity target);

#endif /* SBW_GAME_LOOT_H */
