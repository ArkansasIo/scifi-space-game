// include/library/scaling.h -- level, party, difficulty and PvP scaling.
// See docs/FRAMEWORK.md section 10 and 22.

#ifndef SBW_LIBRARY_SCALING_H
#define SBW_LIBRARY_SCALING_H

    // PartyMult = 1 + (Players - 1) * 0.65, returned x100 for integer math.
    int partyMultiplier(int players);

    // HP = BaseHP * (1 + 0.18 * LevelDiff) * TierMult * PartyMult
    int scaleBossHP(int baseHP, int levelDiff, enemytier tier, int players);

    // DMG = BaseDMG * (1 + 0.12 * LevelDiff) * TierMult * PartyMult^0.6
    int scaleBossDamage(int baseDamage, int levelDiff, enemytier tier, int players);

// EncounterPB = Sum(PlayerPower) * DifficultyFactor
int encounterPowerBudget(int totalPlayerPower, int difficultyFactor);
int difficultyFactor(const difficultmode &mode);

// Normalise a stat for PvP: caps it and applies the PvP conversion.
int normalizeForPvP(int value, int pvpCap, int pvpPercent);

// TotalPower = Character + Fleet + Empire + Technology + Economic
int totalPower(int characterPower, int fleetPower, int empirePower,
               int technologyPower, int economicPower);

// FleetPower = Sum(ShipCombatValue * CrewSkill * TechMultiplier)
int fleetPower(const fleet &f);

// Map a rank multiplier onto a raw enemy stat.
int applyTierMultiplier(int base, enemytier tier);

#endif /* SBW_LIBRARY_SCALING_H */
