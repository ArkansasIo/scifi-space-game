// include/data/stats.h -- stat weight and cap tables.
// See docs/FRAMEWORK.md sections 16.1 and 19.

#ifndef SBW_DATA_STATS_H
#define SBW_DATA_STATS_H

// Fill statCapArray[] with the PvE / PvP hard caps and soft caps.
void initializeStatCaps();

// Fill the per-stat weight table used by the value formulas.
void initializeStatWeights();

// Weight lookup for a named stat ("STR", "Crit Chance", ...), 0 if unknown.
int statWeightByName(const char *name);

// Cap lookup.  Returns the cap for the given context, or -1 if uncapped.
int statCapFor(const char *name, combatcontext context);

// Soft cap for a stat, 0 if it has none.
int statSoftCapFor(const char *name);

#endif /* SBW_DATA_STATS_H */
