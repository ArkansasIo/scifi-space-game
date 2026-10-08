// src/game/combat.c++ -- the combat loop and resolution helpers.
// See docs/FRAMEWORK.md s.3, s.15 and s.20.
//
// This module holds the pure resolution rules.  The engine layer
// (src/engine/combat_engine.c++) owns the fight state machine and calls in
// here; nothing in this file reads or writes global game state.

#include "spacebattlerpg.h"

// HitChance = Accuracy / (Accuracy + Evasion), clamped 5%..95%.
static int hitChancePercent(const statblock &attacker,
                            const statblock &defender)
{
    int accuracy = 70 + attacker.sub.accuracy;

    if (accuracy < 1)
        accuracy = 1;

    int evasion = defender.sub.evasion;
    if (evasion < 0)
        evasion = 0;

    int denominator = accuracy + evasion;
    if (denominator <= 0)
        return 5;

    int chance = (accuracy * 100) / denominator;

    if (chance < 5)
        chance = 5;
    if (chance > 95)
        chance = 95;

    return chance;
}

// Resistance is looked up by damage type through the four-layer model.
static int resistancePercent(const resistancetable &table, damagetype type)
{
    if (type < 0 || type >= DMG_TYPE_COUNT)
        return 0;

    int percent = table.percent[type];

    if (percent > 90)
        percent = 90;

    if (percent < -100)
        percent = -100;

    return percent;
}

combatevent resolveAttack(const statblock &attacker, const statblock &defender,
                          int rawDamage, damagetype type,
                          combatcontext context)
{
    combatevent event;
    memset(&event, 0, sizeof(event));

    event.damage = 0;

    // 1. Hit roll.
    if (randomNumber(100) > hitChancePercent(attacker, defender))
    {
        event.dodged = 1;
        return event;
    }

    // 2. Critical roll.  The cap depends on context: 40% in PvP, 70% in PvE.
    int critCap = (context == CTX_PVP) ? 4000 : 7000;
    int critChance = attacker.combat.critchance;

    if (critChance > critCap)
        critChance = critCap;

    int damage = rawDamage;

    if (critChance > 0 && randomNumber(10000) <= critChance)
    {
        event.crit = 1;
        damage = (damage * attacker.combat.critdamage) / 100;
    }

    // 3. Mitigation: Armor / (Armor + K * AttackerLevel).
    int mitigation = computeMitigation(defender.sub.armor, 1, 50);

    // 4. Resistance and vulnerability.
    int resist = resistancePercent(defender.resist, type);
    int vulnerable = (type >= 0 && type < DMG_TYPE_COUNT)
                         ? defender.resist.vulnerable[type]
                         : 0;

    event.mitigated = computeDamageTaken(damage, mitigation, resist,
                                         vulnerable);

    // True damage ignores the whole mitigation chain.
    if (type == DMG_TRUE)
        event.mitigated = damage;

    if (event.mitigated < 0)
        event.mitigated = 0;

    event.damage = event.mitigated;

    return event;
}

int resolveHeal(const statblock &healer, statblock &target, int healPower,
                int skillCoef, combatcontext context)
{
    // Healing is dampened in PvP to prevent stall comps.
    int bonusHeal = 0;

    if (context == CTX_PVP)
        bonusHeal = -35;

    int heal = computeHealing(healer.sub.healingpower + healPower,
                              skillCoef, bonusHeal, 0);

    target.resource[RES_HP] += heal;

    if (target.resource[RES_HP] > target.resourcemax[RES_HP])
        target.resource[RES_HP] = target.resourcemax[RES_HP];

    return heal;
}

int resolveDoTTick(const statblock &source, const statblock &target,
                   int dotPower, int skillCoef, damagetype type)
{
    int mitigation = computeMitigation(target.sub.armor, 1, 50);

    // A DoT uses the DoT power, not raw attack.
    int tick = computeDoTTick(source.combat.dotpower + dotPower, skillCoef,
                              mitigation);

    // Elemental resistances still apply to the tick.
    int resist = resistancePercent(target.resist, type);

    if (resist > 0)
        tick -= (tick * resist) / 100;

    if (tick < 0)
        tick = 0;

    return tick;
}

void runCombatTurn(statblock &player, statblock &enemy)
{
    // A single exchange, used by callers that drive combat themselves.
    int playerDamage = player.combat.baseattack + randomNumber(player.sub.physicalpower + 1);

    combatevent playerEvent = resolveAttack(player, enemy, playerDamage,
                                            DMG_PHYSICAL, CTX_PVE);

    enemy.resource[RES_HP] -= playerEvent.damage;

    if (enemy.resource[RES_HP] <= 0)
    {
        enemy.resource[RES_HP] = 0;
        return;
    }

    int enemyDamage = enemy.combat.baseattack + randomNumber(enemy.sub.physicalpower + 1);

    combatevent enemyEvent = resolveAttack(enemy, player, enemyDamage,
                                           DMG_PHYSICAL, CTX_PVE);

    player.resource[RES_HP] -= enemyEvent.damage;

    if (player.resource[RES_HP] < 0)
        player.resource[RES_HP] = 0;
}
