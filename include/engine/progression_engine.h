// include/engine/progression_engine.h -- the progression engine.
//
// Character, fleet, faction and empire growth, plus the attribute spend and
// talent hooks the UI calls into.

#ifndef SBW_ENGINE_PROGRESSION_ENGINE_H
#define SBW_ENGINE_PROGRESSION_ENGINE_H

struct progressionstate
{
    int level;
    int experience;
    int pendingAttributePoints;
    int pendingSkillPoints;
    int characterPower;
    int fleetRank;
    int factionReputation;
    int empireRank;
    int techLevel;
};

/* ---------------- lifecycle ---------------- */

int progressionEngineInit();
int progressionEngineStart();
int progressionEngineUpdate(int ticks);
void progressionEngineStop();
void progressionEngineShutdown();

/* ---------------- runtime ---------------- */

void clearProgression(progressionstate &state);

// XP required for the next level from the current one.
int progressionXPToNext(const progressionstate &state);

// Award XP.  Returns levels gained.
int progressionGrantXP(progressionstate &state, int amount);

// Spend a pending attribute point into a core attribute.
int progressionSpendAttribute(coreattributes &attrs, int attributeId);

// Spend a pending skill point.
int progressionSpendSkill(progressionstate &state, int skillId);

/* ---------------- derived power ---------------- */

// Recompute character power from a stat block.
int progressionCharacterPower(const statblock &stats);

// Fleet rank from victories.
int progressionFleetRank(int fleetVictories);

// Faction reputation tier and empire rank.
int progressionReputationTier(int reputation);
int progressionEmpireRank(int systemsControlled);

/* ---------------- applied growth ---------------- */

// Apply a level's worth of growth to a stat block.
void progressionApplyLevel(statblock &stats, int newLevel);

// Rebuild a stat block from scratch at the given level.
void progressionBuildStats(statblock &stats, int level, int classId);

/* ---------------- reporting ---------------- */

void progressionShowSummary(const progressionstate &state);

#endif /* SBW_ENGINE_PROGRESSION_ENGINE_H */
