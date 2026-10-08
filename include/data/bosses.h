// include/data/bosses.h -- boss metadata: phases, arena, enrage, loot.
// See docs/FRAMEWORK.md section 8.

#ifndef SBW_DATA_BOSSES_H
#define SBW_DATA_BOSSES_H

// Fill bossArray[] with the boss roster (dungeon, world and raid bosses).
void initializeBosses();

// Look up a boss by name.  Returns index, or -1.
int findBoss(const char *name);

// Total effective HP across all of a boss's phases.
int bossTotalHP(const bossdata &boss);

// Count the mechanics modules a boss uses across all phases.
int bossMechanicCount(const bossdata &boss);

// True if the boss is immune to a damage type in the given phase.
int bossImmunity(const bossdata &boss, int phaseIndex, damagetype type);

// Arena hazard count for a boss (index into the encounter hazard table).
int bossArenaHazardCount(const bossdata &boss);

#endif /* SBW_DATA_BOSSES_H */
