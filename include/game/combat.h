// include/game/combat.h -- the combat loop and resolution helpers.
// See docs/FRAMEWORK.md sections 3, 15 and 20.

#ifndef SBW_GAME_COMBAT_H
#define SBW_GAME_COMBAT_H

// Outcome of one resolved attack.
struct combatevent
{
    int damage;
    int mitigated;
    int crit;
    int blocked;
    int parried;
    int dodged;
    int targetHp;
    int targetDown;
};

// Run one full attack: hit roll -> crit roll -> mitigation -> resistances.
combatevent resolveAttack(const statblock &attacker, const statblock &defender,
                          int rawDamage, damagetype type, combatcontext context);

// Heal a target, honouring bonus-heal and heal-reduction modifiers.
int resolveHeal(const statblock &healer, statblock &target, int healPower,
                int skillCoef, combatcontext context);

// Apply a damage-over-time tick.
int resolveDoTTick(const statblock &source, const statblock &target,
                   int dotPower, int skillCoef, damagetype type);

// The turn loop used by the battle generator.
void runCombatTurn(statblock &player, statblock &enemy);

// Bridge between the legacy `player` / `enemyships` structs and statblock.
statblock buildPlayerStatBlock();
statblock buildEnemyStatBlock(int enemyIndex);

#endif /* SBW_GAME_COMBAT_H */
