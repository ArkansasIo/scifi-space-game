// src/library/stats.c++ -- the stat model and its baseline formulas.
// See docs/FRAMEWORK.md sections 1-5 and 15.

#include "spacebattlerpg.h"

void clearStatBlock(statblock &stats)
{
    memset(&stats, 0, sizeof(stats));
}

void deriveSubAttributes(statblock &stats)
{
    const coreattributes &a = stats.attributes;

    // Offensive derivation.
    stats.sub.physicalpower = (a.str * 5) / 2; // STR * 2.5
    stats.sub.spellpower = (a.intel * 5) / 2;  // INT * 2.5
    stats.sub.healingpower = (a.wis * 2) + (a.intel / 2);
    stats.sub.critchance = computeCritChance(a.dex, 0);
    stats.sub.critdamage = 150 + (a.lck / 2); // base 150% + luck
    stats.sub.accuracy = 70 + (a.dex * 2);
    stats.sub.evasion = 5 + (a.dex / 2);

    // Survival derivation.
    stats.sub.maxhp = computeMaxHP(a.vit, a.con, 0);
    stats.sub.hpregen = 1 + (a.vit / 10) + (a.spi / 20);
    stats.sub.armor = a.con + (a.str / 2);
    stats.sub.magicresist = a.wil + (a.spi / 2);
    stats.sub.tenacity = a.wil;
    stats.sub.poise = a.con / 2;

    // Tempo / control.
    stats.sub.controlpower = 50 + (a.cha / 2) + (a.wil / 4);
    stats.sub.shieldpower = a.spi + (a.intel / 4);
}

void deriveCombatStats(statblock &stats)
{
    stats.combat.baseattack = stats.sub.physicalpower + (stats.sub.spellpower / 2);
    stats.combat.skillpower = 100;
    stats.combat.critchance = stats.sub.critchance;
    stats.combat.critdamage = stats.sub.critdamage;
    stats.combat.vulnerabilitydmg = 0;
    stats.combat.dotpower = stats.sub.spellpower / 2;
    stats.combat.executedmg = 0;

    stats.combat.mitigation =
        computeMitigation(stats.sub.armor, 1, 50);
    stats.combat.blockchance = 0;
    stats.combat.blockvalue = stats.sub.armor / 4;
    stats.combat.parrychance = 0;
    stats.combat.dodgechance = stats.sub.evasion;

    stats.combat.movespeed = 100;
    stats.combat.cdr = 0;
    stats.combat.gcd = 100; // 1.00 second, x100
}

void applyStatCaps(statblock &stats, combatcontext context)
{
    // Walk the cap table and clamp anything that names a live stat.
    for (int i = 0; i < statCapCount; ++i)
    {
        const statcap &cap = statCapArray[i];
        int limit = (context == CTX_PVP) ? cap.pvpCap : cap.pveCap;

        if (limit <= 0)
            continue;

        // Crit chance is the stat most needing a hard cap.
        if (strcmp(cap.name, "Crit Chance") == 0)
        {
            if (stats.combat.critchance > limit)
                stats.combat.critchance = limit;
        }
        else if (strcmp(cap.name, "Cooldown Reduction") == 0)
        {
            if (stats.combat.cdr > limit)
                stats.combat.cdr = limit;
        }
        else if (strcmp(cap.name, "Damage Reduction") == 0)
        {
            if (stats.sub.damagereduction > limit)
                stats.sub.damagereduction = limit;
        }
    }
}

/* ---------------- baseline formulas ---------------- */

int computeMaxHP(int vit, int con, int gearHP)
{
    return (vit * 15) + (con * 10) + gearHP;
}

int computePhysicalDamage(int str, int weaponMultiplier)
{
    // weaponMultiplier is a percentage (100 = 1.00x).
    return ((str * 5) / 2) * weaponMultiplier / 100;
}

int computeCritChance(int dex, int gearCrit)
{
    // DEX * 0.04, converted to basis points, plus flat gear crit.
    int bp = (dex * 4);
    return bp + gearCrit;
}

int computeMitigation(int armor, int attackerLevel, int k)
{
    if (armor <= 0)
        return 0;

    if (attackerLevel <= 0)
        attackerLevel = 1;

    int denominator = armor + (k * attackerLevel);
    if (denominator <= 0)
        return 0;

    // Return basis points (0..10000) so the caller keeps integer maths.
    return (armor * 10000) / denominator;
}

int computeDamageTaken(int rawDamage, int mitigationBp, int resistPercent,
                       int vulnerabilityPercent)
{
    if (rawDamage <= 0)
        return 0;

    int afterMitigation = rawDamage;

    if (mitigationBp > 0)
    {
        if (mitigationBp > 9500)
            mitigationBp = 9500; // never fully immune to raw damage
        afterMitigation = rawDamage - (rawDamage * mitigationBp) / 10000;
    }

    if (resistPercent > 0)
    {
        if (resistPercent > 90)
            resistPercent = 90;
        afterMitigation -= (afterMitigation * resistPercent) / 100;
    }

    if (vulnerabilityPercent > 0)
        afterMitigation += (afterMitigation * vulnerabilityPercent) / 100;

    if (afterMitigation < 0)
        afterMitigation = 0;

    return afterMitigation;
}

int computeHealing(int healingPower, int skillCoef, int bonusHeal,
                   int healReduction)
{
    int heal = (healingPower * skillCoef) / 100;

    if (bonusHeal > 0)
        heal += (heal * bonusHeal) / 100;

    if (healReduction > 0)
        heal -= (heal * healReduction) / 100;

    if (heal < 0)
        heal = 0;

    return heal;
}

int computeDoTTick(int dotPower, int skillCoef, int mitigationBp)
{
    int tick = (dotPower * skillCoef) / 100;

    if (mitigationBp > 0)
    {
        if (mitigationBp > 9000)
            mitigationBp = 9000;
        tick -= (tick * mitigationBp) / 10000;
    }

    if (tick < 1 && dotPower > 0)
        tick = 1; // a DoT that lands always ticks for something

    return tick;
}
