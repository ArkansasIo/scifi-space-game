// src/library/itemvalue.c++ -- item power and value formulas.
// See docs/FRAMEWORK.md section 16.

#include "spacebattlerpg.h"
#include <cmath>

int statValuePU(int rawStat, int weight)
{
    // Weight is x100 (100 = 1.00), so the result stays in whole units.
    return (rawStat * weight) / 100;
}

int softCapCurve(int rawStat, int softCap)
{
    if (rawStat <= 0)
        return 0;

    if (softCap <= 0)
        return rawStat;

    // EffectiveStat = raw / (raw + softCap) * softCap
    int denominator = rawStat + softCap;
    if (denominator <= 0)
        return 0;

    return (rawStat * softCap) / denominator;
}

int diminishingCurve(int rawStat, int cap, int k)
{
    if (rawStat <= 0)
        return 0;

    if (cap <= 0)
        return 0;

    if (k <= 0)
        k = 1;

    // EffectiveValue = Cap * (1 - e^(-raw / K))
    double exponent = -(double)rawStat / (double)k;
    double effective = (double)cap * (1.0 - exp(exponent));

    return (int)(effective + 0.5);
}

int weaponPowerPvE(int baseDamage, int speedFactor, int attributeScaling,
                   int elementalScaling, int pveMultiplier)
{
    // WeaponPower = (BaseDamage * SpeedFactor% + Attr + Element) * PvE%
    int core = (baseDamage * speedFactor) / 100;
    core += attributeScaling;
    core += elementalScaling;

    return (core * pveMultiplier) / 100;
}

int weaponPowerPvP(int normalizedDamage, int attributeScaling, int pvpStatScale,
                   int pvpDampening)
{
    // Normalized damage is not scaled by weapon speed; attribute scaling is
    // deliberately reduced (0.65x) and the whole result dampened (0.75-0.85).
    int scaledAttributes = (attributeScaling * pvpStatScale) / 100;
    int core = normalizedDamage + scaledAttributes;

    return (core * pvpDampening) / 100;
}

int totalDefensivePower(int hp, int armor, int resist, int survivalMultiplier)
{
    // TDP = (HP*0.4 + Armor*0.3 + Resist*0.3) * SurvivalMultiplier
    int tdp = (hp * 40) / 100;
    tdp += (armor * 30) / 100;
    tdp += (resist * 30) / 100;

    return (tdp * survivalMultiplier) / 100;
}

int procValue(int magnitude, int uptimePercent, int impactWeight)
{
    // ProcValue = magnitude * uptime% * impactWeight%
    int value = (magnitude * uptimePercent) / 100;
    value = (value * impactWeight) / 100;

    return value;
}

int itemPower(const itemdata &item)
{
    int total = 0;

    for (int i = 0; i < item.pveLineCount; ++i)
        total += statValuePU(item.pveLines[i].raw, item.pveLines[i].weight);

    total += item.procValuePU;
    total += item.setBonusValuePU;

    return total;
}

int itemPowerFor(const itemdata &item, combatcontext context)
{
    if (context == CTX_PVE)
        return item.itemPowerPvE;

    return item.itemPowerPvP;
}

void recalcItemPower(itemdata &item)
{
    // PvE reads the full table.
    int pve = 0;
    for (int i = 0; i < item.pveLineCount; ++i)
    {
        int value = statValuePU(item.pveLines[i].raw, item.pveLines[i].weight);
        item.pveLines[i].valuePU = value;
        pve += value;
    }
    pve += item.procValuePU;
    pve += item.setBonusValuePU;
    item.itemPowerPvE = pve;

    // PvP reads the normalized table and ignores procs entirely.
    int pvp = 0;
    for (int i = 0; i < item.pvpLineCount; ++i)
    {
        int value = statValuePU(item.pvpLines[i].raw, item.pvpLines[i].weight);
        item.pvpLines[i].valuePU = value;
        pvp += value;
    }
    pvp += item.setBonusValuePU;
    item.itemPowerPvP = pvp;
}

int buffValue(int statIncrease, int duration, int targetCount,
              int contextMultiplier)
{
    // BuffValue = statIncrease% * duration * targets * context%
    int value = (statIncrease * duration) / 100;
    value *= targetCount;
    value = (value * contextMultiplier) / 100;

    return value;
}

int ccValue(int baseDuration, int controlWeight, int drMultiplier,
            int targetImportance)
{
    // CCValue = baseDuration * controlWeight% * DR% * importance%
    int value = (baseDuration * controlWeight) / 100;
    value = (value * drMultiplier) / 100;
    value = (value * targetImportance) / 100;

    return value;
}
