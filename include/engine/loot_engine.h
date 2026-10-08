// include/engine/loot_engine.h -- the loot engine.
//
// Rolls drop tables, tracks pity timers, applies reward multipliers and
// distributes to the party.

#ifndef SBW_ENGINE_LOOT_ENGINE_H
#define SBW_ENGINE_LOOT_ENGINE_H

enum lootdistribution
{
    LOOT_PERSONAL = 0,
    LOOT_NEED_GREED,
    LOOT_MASTER,
    LOOT_ROUND_ROBIN
};

struct lootruntime
{
    int pity[16];
    int rollsSinceRare;
    int totalRolls;
    int rareDrops;
    int luckApplied;
};

/* ---------------- lifecycle ---------------- */

int lootEngineInit();
int lootEngineStart();
int lootEngineUpdate(int ticks);
void lootEngineStop();
void lootEngineShutdown();

/* ---------------- runtime ---------------- */

void clearLootRuntime(lootruntime &rt);

// Roll a table for a group, writing drops into `out`.
int lootRoll(lootruntime &rt, int tableIndex, int difficultyTier,
             int partySize, int luck, lootresult out[], int outMax);

// Roll a single entry by rarity weight, honouring the pity timer.
gearrarity lootRollRarity(lootruntime &rt, int tableIndex, int luck);

// Advance all pity timers after a roll.
void lootUpdatePity(lootruntime &rt, gearrarity rolled, gearrarity target);

// Reward multiplier for these clear conditions.
int lootRewardMultiplier(int difficultyTier, enemytier tier,
                         int clearTimePercent, int noDeath);

// Bounded luck so a stacked bonus can never break the economy.
int lootBoundLuck(int rawLuck);

// Distribute a drop list according to a rule.  Returns the recipient index.
int lootDistribute(lootdistribution rule, int partySize, int dropIndex);

/* ---------------- reporting ---------------- */

void lootShowSummary(const lootruntime &rt);
int lootDropCount(const lootresult out[], int count, gearrarity rarity);

#endif /* SBW_ENGINE_LOOT_ENGINE_H */
