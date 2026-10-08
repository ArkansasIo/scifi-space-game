// include/game/progression.h -- character, fleet, faction and empire growth.
// See docs/FRAMEWORK.md section 22.11.

#ifndef SBW_GAME_PROGRESSION_H
#define SBW_GAME_PROGRESSION_H

// XP required to reach the given level.
int xpForLevel(int level);

// Award XP and return the number of levels gained.
int grantExperience(player &who, int amount);

// Spend attribute points into a core attribute.  Returns 1 on success.
int spendAttributePoint(coreattributes &attrs, int attributeId, int points);

// Commander rank from fleet victories.
int commanderRank(int fleetVictories);

// Faction reputation tier.
int reputationTier(int reputation);

// Empire rank from controlled territory.
int empireRank(int systemsControlled);

// Which victory path the empire is closest to.
victorytype leadingVictory(const faction &who, int systemsControlled,
                           int techLevel);

// ResearchTime = TechCost / (ScienceOutput * ScientistSkill)
int researchTime(int techCost, int scienceOutput, int scientistSkill);

#endif /* SBW_GAME_PROGRESSION_H */
