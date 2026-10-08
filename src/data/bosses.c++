// src/data/bosses.c++ -- boss metadata: phases, arena, enrage, loot.
// See docs/FRAMEWORK.md s.8.

#include "spacebattlerpg.h"

// The boss roster.  Each entry carries a phase plan with HP thresholds,
// immunity masks, enrage rules and a loot table reference.
static const bossdata bossSeeds[] = {
    /* Dungeon bosses */
    {"The Ash Tyrant", TIER_DUNGEON_BOSS, 30, 180000, 3, {{"Cinders", 100, 0, 4, -1, 0}, {"Pyroclasm", 60, 0, 6, 0, 0}, {"Final Conflagration", 25, 0, 8, 0, 0}}, 2, 12, 480, 1, 100, "Titan, Undead"},

    {"Frost Warden", TIER_DUNGEON_BOSS, 32, 200000, 2, {{"Deep Cold", 100, 0, 4, -1, 0}, {"Absolute Zero", 45, 120, 7, 1, 0}}, 2, 8, 420, 1, 100, "Elemental"},

    /* World bosses */
    {"Warmind Colossus", TIER_WORLD_BOSS, 45, 900000, 3, {{"Boot Sequence", 100, 0, 5, -1, 0}, {"Grid Online", 70, 0, 7, 2, 0}, {"Overload", 30, 0, 10, 2, 0}}, 3, 10, 600, 2, 100, "Machine, Titan"},

    /* Raid bosses */
    {"The Hive Sovereign", TIER_RAID_BOSS, 60, 2500000, 4, {{"Brood", 100, 0, 6, -1, 0}, {"Swarm Tide", 75, 0, 8, 3, 0}, {"Hive Mind", 50, 0, 10, 3, 0}, {"Queen's Fall", 20, 0, 12, 3, 0}}, 4, 15, 720, 3, 100, "Beast, Hive"},

    {"The Divine Vanguard", TIER_RAID_BOSS, 65, 3200000, 3, {{"Spearhead", 100, 0, 7, -1, 0}, {"Imperial Line", 65, 0, 9, 4, 0}, {"Divine Right", 30, 0, 12, 4, 0}}, 4, 18, 780, 3, 100, "Humanoid, Divine"},

    /* Mythic */
    {"The Ascendant", TIER_MYTHIC, 80, 9000000, 4, {{"Descent", 100, 0, 8, -1, 0}, {"Shattered Sky", 80, 0, 10, 5, 0}, {"The Gate", 55, 0, 12, 5, 0}, {"Last Star", 20, 0, 15, 5, 0}}, 5, 22, 900, 4, 100, "Divine, Titan, Undead"}};

static const int bossSeedCount =
    (int)(sizeof(bossSeeds) / sizeof(bossSeeds[0]));

void initializeBosses()
{
    bossCount = 0;

    for (int i = 0; i < bossSeedCount && i < MAX_BOSSES; ++i)
    {
        bossArray[bossCount] = bossSeeds[i];
        bossCount++;
    }
}

int findBoss(const char *name)
{
    if (!name)
        return -1;

    for (int i = 0; i < bossCount; ++i)
    {
        if (strcmp(bossArray[i].name, name) == 0)
            return i;
    }

    return -1;
}

int bossTotalHP(const bossdata &boss)
{
    // Phases split the HP pool; the last phase cannot empty the pool, so the
    // effective total is the pool itself plus the phase padding.
    int total = boss.hpPool;

    for (int i = 0; i < boss.phaseCount && i < 4; ++i)
        total += boss.phases[i].abilityCount * 100;

    return total;
}

int bossMechanicCount(const bossdata &boss)
{
    int count = 0;

    for (int i = 0; i < boss.phaseCount && i < 4; ++i)
        count += boss.phases[i].abilityCount;

    return count;
}

int bossImmunity(const bossdata &boss, int phaseIndex, damagetype type)
{
    if (phaseIndex < 0 || phaseIndex >= boss.phaseCount || phaseIndex >= 4)
        return 0;

    int mask = boss.phases[phaseIndex].immuneMask;

    if (mask <= 0)
        return 0;

    return (mask & (1 << (int)type)) ? 1 : 0;
}

int bossArenaHazardCount(const bossdata &boss)
{
    return boss.arenaHazardCount;
}

// Pick the phase that applies at a given HP percentage.  Phases are ordered
// from full HP down, so the first phase whose threshold the boss has reached
// is the active one.
const bossphase *currentBossPhase(const bossdata &boss, int hpPercent)
{
    const bossphase *chosen = 0;

    for (int i = 0; i < boss.phaseCount && i < 4; ++i)
    {
        if (hpPercent <= boss.phases[i].hpThreshold)
            chosen = &boss.phases[i];
    }

        // Above every threshold, the opening phase is active.
        if (!chosen && boss.phaseCount > 0)
            chosen = &boss.phases[0];

        return chosen;
    }

int bossShouldEnrage(const bossdata &boss, int elapsedTicks)
{
    if (boss.hardEnrageTime <= 0)
        return 0;

    return (elapsedTicks >= boss.hardEnrageTime) ? 1 : 0;
}

int softEnrageBonus(const bossdata &boss, int elapsedTicks)
{
    if (boss.softEnrageRate <= 0)
        return 0;

    // +softEnrageRate% damage for every 15 seconds elapsed.
    int intervals = elapsedTicks / 15;

    if (intervals <= 0)
        return 0;

    return intervals * boss.softEnrageRate;
}
