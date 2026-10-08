// include/library/stats.h -- the stat model: core attributes, derived
// sub-attributes, combat stats and the resistance table.
//
// See docs/FRAMEWORK.md sections 1-5 and 15.

#ifndef SBW_LIBRARY_STATS_H
#define SBW_LIBRARY_STATS_H

    // Zero every field of a stat block.
    void clearStatBlock(statblock &stats);

// Derive sub-attributes from core attributes (the level-up / gear baseline).
void deriveSubAttributes(statblock &stats);

// Roll the derived sub-attributes up into the final combat stats.
void deriveCombatStats(statblock &stats);

// Apply soft caps and hard caps to a stat block.
void applyStatCaps(statblock &stats, combatcontext context);

// --- baseline formulas (section 15 / 22) ---

// MaxHP = (VIT * 15) + (CON * 10) + gearHP
int computeMaxHP(int vit, int con, int gearHP);

// PhysicalDamage = (STR * 2.5) * weaponMultiplier
int computePhysicalDamage(int str, int weaponMultiplier);

// CritChance = (DEX * 0.04) + gearCrit, returned in basis points (0..10000).
int computeCritChance(int dex, int gearCrit);

// Mitigation = Armor / (Armor + K * AttackerLevel), basis points.
int computeMitigation(int armor, int attackerLevel, int k);

// DamageTaken = raw * (1 - mitigation) * (1 - resistPercent) * vulnerability
int computeDamageTaken(int rawDamage, int mitigationBp, int resistPercent,
                       int vulnerabilityPercent);

// Healing = power * skillCoef% * (1 + bonusHeal%) * (1 - healReduction%)
int computeHealing(int healingPower, int skillCoef, int bonusHeal,
                   int healReduction);

// TickDamage = (DoTPower * SkillCoef%) * (1 - mitigation)
int computeDoTTick(int dotPower, int skillCoef, int mitigationBp);

#endif /* SBW_LIBRARY_STATS_H */
