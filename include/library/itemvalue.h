// include/library/itemvalue.h -- item power / value formulas.
// See docs/FRAMEWORK.md section 16.

#ifndef SBW_LIBRARY_ITEMVALUE_H
#define SBW_LIBRARY_ITEMVALUE_H

// StatValuePU = raw * weight * scalingCurve
int statValuePU(int rawStat, int weight);

// Soft-cap curve: EffectiveStat = raw / (raw + softCap) * softCap
int softCapCurve(int rawStat, int softCap);

// Diminishing returns: Cap * (1 - e^(-raw / K))
int diminishingCurve(int rawStat, int cap, int k);

// Weapon power, PvE and PvP variants (section 16.3).
int weaponPowerPvE(int baseDamage, int speedFactor, int attributeScaling,
                   int elementalScaling, int pveMultiplier);
int weaponPowerPvP(int normalizedDamage, int attributeScaling, int pvpStatScale,
                   int pvpDampening);

// Total Defensive Power: (HP*0.4 + Armor*0.3 + Resist*0.3) * survival.
int totalDefensivePower(int hp, int armor, int resist, int survivalMultiplier);

// ProcValue = magnitude * uptime% * impactWeight%
int procValue(int magnitude, int uptimePercent, int impactWeight);

// ItemPower = Sum(StatValuePU) + ProcValue + SetBonusValue
int itemPower(const itemdata &item);
int itemPowerFor(const itemdata &item, combatcontext context);

// Recompute itemPowerPvE / itemPowerPvP on an item from its stat lines.
void recalcItemPower(itemdata &item);

// BuffValue = statIncrease * duration * targetCount * contextMultiplier
int buffValue(int statIncrease, int duration, int targetCount,
              int contextMultiplier);

// CCValue = baseDuration * controlWeight * drMultiplier * targetImportance
int ccValue(int baseDuration, int controlWeight, int drMultiplier,
            int targetImportance);

#endif /* SBW_LIBRARY_ITEMVALUE_H */
