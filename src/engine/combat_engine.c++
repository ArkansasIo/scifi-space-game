// src/engine/combat_engine.c++ -- the combat engine.
//
// Owns the fight state machine and resolves every action.  The pipeline is:
//
//   CMB_SETUP -> CMB_TURN_START -> CMB_SELECT_ACTION -> CMB_RESOLVE
//             -> CMB_TURN_END -> (repeat) -> CMB_VICTORY / CMB_DEFEAT
//
// Damage flows through the same helpers the specification describes:
//   hit roll -> crit roll -> mitigation -> resistances -> apply
// See docs/FRAMEWORK.md s.15.

#include "spacebattlerpg.h"
#include "engine/combat_engine.h"
#include "engine/effect_engine.h"
#include "engine/ai_engine.h"
#include "ui/display.h"

/* ---------------- internal state ---------------- */

static int g_combatRunning = 0;
static int g_playerSideWon = 0;

/* ---------------- combatant helpers ---------------- */

static void clearCombatant(combatant &who)
{
    memset(&who, 0, sizeof(who));

    for (int i = 0; i < 16; ++i)
        clearActiveEffect(who.effects[i]);
}

static void syncCombatantHealth(combatant &who)
{
    if (who.hp > who.maxHp)
        who.hp = who.maxHp;

    who.alive = (who.hp > 0) ? 1 : 0;
}

/* ---------------- lifecycle ---------------- */

int combatEngineInit()
{
    g_playerSideWon = 0;
    return 1;
}

int combatEngineStart()
{
    g_combatRunning = 1;
    return 1;
}

int combatEngineUpdate(int ticks)
{
    (void)ticks;

    if (!g_combatRunning)
        return 0;

    return 1;
}

void combatEngineStop()
{
    g_combatRunning = 0;
}

void combatEngineShutdown()
{
    g_combatRunning = 0;
    g_playerSideWon = 0;
}

/* ---------------- state ---------------- */

void clearCombatState(combatstate &state)
{
    memset(&state, 0, sizeof(state));

    clearCombatant(state.player);
    clearCombatant(state.enemy);

    state.phase = CMB_IDLE;
}

void beginCombat(combatstate &state, int enemyIndex, int pvp)
{
    clearCombatState(state);

    if (enemyIndex < 0 || enemyIndex >= 17)
        return;

    // Player side, from the live character and the RPG stat model.
    state.player.stats = buildPlayerStatBlock();
    strncpy(state.player.name, charactership.name,
            sizeof(state.player.name) - 1);
    state.player.team = 0;
    state.player.hp = charactership.sp;
    state.player.maxHp = charactership.sp;
    state.player.shield = charactership.shield;
    state.player.alive = 1;

    // Enemy side, from the fleet table.
    state.enemy.stats = buildEnemyStatBlock(enemyIndex);
    strncpy(state.enemy.name, enemyshipsArray[enemyIndex].name,
            sizeof(state.enemy.name) - 1);
    state.enemy.team = 1;
    state.enemy.hp = enemyshipsArray[enemyIndex].sp;
    state.enemy.maxHp = enemyshipsArray[enemyIndex].sp;
    state.enemy.shield = enemyshipsArray[enemyIndex].sif;
    state.enemy.alive = 1;

    state.pvp = pvp ? 1 : 0;

    // Initiative decides who opens the fight.
    state.player.initiative = rollInitiative(state.player);
    state.enemy.initiative = rollInitiative(state.enemy);

    state.round = 1;
    state.turn = (state.player.initiative >= state.enemy.initiative) ? 0 : 1;

    state.phase = CMB_SETUP;

    combatLogLine(state, "The battle begins.");
}

int rollInitiative(const combatant &who)
{
    // Initiative = Speed + TacticalSkill + EquipmentBonus, with a d10 roll.
    int base = who.stats.combat.movespeed / 10;
    base += who.stats.attributes.dex / 2;
    base += who.stats.combat.cdr / 20;

    return base + randomNumber(10);
}

/* ---------------- phase machine ---------------- */

combatphase advanceCombatPhase(combatstate &state)
{
    switch (state.phase)
    {
    case CMB_SETUP:
        state.phase = CMB_TURN_START;
        break;

    case CMB_TURN_START:
        state.phase = CMB_SELECT_ACTION;
        break;

    case CMB_SELECT_ACTION:
        state.phase = CMB_RESOLVE;
        break;

    case CMB_RESOLVE:
        state.phase = CMB_TURN_END;
        break;

    case CMB_TURN_END:
        if (combatIsOver(state))
        {
            state.phase = (combatWinner(state) == 0) ? CMB_VICTORY
                                                     : CMB_DEFEAT;
        }
        else
        {
            state.turn = 1 - state.turn;

            if (state.turn == 0)
                state.round++;

            state.phase = CMB_TURN_START;
        }
        break;

    default:
        break;
    }

    return state.phase;
}

int performTurn(combatstate &state)
{
    return resolveAction(state, state.turn, ACT_ATTACK);
}

int performEnemyTurn(combatstate &state)
{
    // The AI picks its action; for now the choice is attack or defend.
    aiworld world;
    clearAIWorld(world);

    world.currentTick = state.ticksElapsed;
    world.playerHpPercent = (state.player.maxHp > 0)
                                ? (state.player.hp * 100) / state.player.maxHp
                                : 0;
    world.distanceToTarget = 1;
    world.alliesAlive = 0;

    combataction action = ACT_ATTACK;

    if (world.playerHpPercent < 20 && randomNumber(100) <= 25)
        action = ACT_DEFEND; // low player HP does not change much, but it
                             // keeps the AI from being purely robotic

    return resolveAction(state, 1, action);
}

/* ---------------- action resolution ---------------- */

// Hit chance: Accuracy / (Accuracy + Evasion), clamped 5%..95%.
static int rollHit(const statblock &attacker, const statblock &defender)
{
    int accuracy = 70 + attacker.sub.accuracy;
    int evasion = defender.sub.evasion;

    if (accuracy < 1)
        accuracy = 1;

    int denominator = accuracy + evasion;
    if (denominator <= 0)
        denominator = 1;

    int chance = (accuracy * 100) / denominator; // percent

    if (chance < 5)
        chance = 5;
    if (chance > 95)
        chance = 95;

    return (randomNumber(100) <= chance) ? 1 : 0;
}

// Crit chance comes straight off the stat block, capped by context.
static int rollCrit(const statblock &attacker, int pvp)
{
    int cap = pvp ? 4000 : 7000; // basis points
    int chance = attacker.combat.critchance;

    if (chance > cap)
        chance = cap;

    if (chance <= 0)
        return 0;

    // Convert basis points to a 1..10000 roll.
    return (randomNumber(10000) <= chance) ? 1 : 0;
}

int resolveAction(combatstate &state, int actorSide, combataction action)
{
    combatant &actor = (actorSide == 0) ? state.player : state.enemy;
    combatant &target = (actorSide == 0) ? state.enemy : state.player;

    if (!actor.alive || !target.alive)
        return 0;

    if (action == ACT_ATTACK)
    {
        if (!rollHit(actor.stats, target.stats))
        {
            combatLogLine(state, "The attack misses.");
            audioPlaySFX(SFX_DODGE);
            return 0;
        }

        // Base damage from the attacker's attack power.
        int raw = actor.stats.combat.baseattack;
        raw += randomNumber(actor.stats.sub.physicalpower + 1);

        // Critical hits multiply the raw damage.
        int crit = rollCrit(actor.stats, state.pvp);
        if (crit)
        {
            raw = (raw * actor.stats.combat.critdamage) / 100;
            audioPlaySFX(SFX_CRITICAL_HIT);
        }

        combatevent event = resolveAttack(actor.stats, target.stats, raw,
                                          DMG_PHYSICAL,
                                          state.pvp ? CTX_PVP : CTX_PVE);

        // Shields absorb first.
        int applied = event.damage;

        if (target.shield > 0)
        {
            int absorbed = (applied < target.shield) ? applied : target.shield;
            target.shield -= absorbed;
            applied -= absorbed;

            if (absorbed > 0)
                audioPlaySFX(SFX_SHIELD_HIT);

            if (target.shield <= 0)
                audioPlaySFX(SFX_SHIELD_DOWN);
        }

        target.hp -= applied;
        syncCombatantHealth(target);

        if (actorSide == 0)
        {
            state.totalDamageDealt += applied;
            targetScore.damage_done += applied;
        }
        else
        {
            state.totalDamageTaken += applied;
            targetScore.damage_taken += applied;
            audioPlaySFX(SFX_PLAYER_DAMAGED);
        }

        if (actorSide == 0)
            charactership.sp = target.hp;
        else
            charactership.sp = actor.hp;
    }
    else if (action == ACT_DEFEND)
    {
        // Defending grants a small shield buffer for the round.
        actor.shield += actor.stats.sub.armor / 2;
        combatLogLine(state, "Raises shields.");
    }
    else if (action == ACT_FLEE)
    {
        state.phase = CMB_FLED;
    }

    targetScore.actions_taken++;

    return state.totalDamageDealt;
}

/* ---------------- effects ---------------- */

void tickCombatEffects(combatstate &state)
{
    effectruntime rt;
    clearEffectRuntime(rt);

    rt.currentTick = state.ticksElapsed;

    int dotOnPlayer = effectDoTDamage(state.player.effects, 16,
                                      state.player.stats.combat.mitigation);
    int dotOnEnemy = effectDoTDamage(state.enemy.effects, 16,
                                     state.enemy.stats.combat.mitigation);

    if (dotOnPlayer > 0)
    {
        state.player.hp -= dotOnPlayer;
        syncCombatantHealth(state.player);
    }

    if (dotOnEnemy > 0)
    {
        state.enemy.hp -= dotOnEnemy;
        syncCombatantHealth(state.enemy);
    }

    effectTick(rt, state.player.effects, 16);
    effectTick(rt, state.enemy.effects, 16);
}

int applyCombatEffect(combatstate &state, int side, int definitionIndex)
{
    if (side == 0)
    {
        int slot = applyEffect(state.player.effects, 16, definitionIndex, 0);
        if (slot >= 0)
            audioPlaySFX(SFX_BUFF_APPLIED);
        return slot;
    }

    int slot = applyEffect(state.enemy.effects, 16, definitionIndex, 1);
    if (slot >= 0)
        audioPlaySFX(SFX_DEBUFF_APPLIED);

    return slot;
}

/* ---------------- queries ---------------- */

int combatIsOver(const combatstate &state)
{
    if (state.phase == CMB_FLED)
        return 1;

    if (!state.player.alive || !state.enemy.alive)
        return 1;

    return 0;
}

int combatWinner(const combatstate &state)
{
    // 0 = player side, 1 = enemy side, -1 = unresolved.
    if (state.phase == CMB_FLED)
        return 1;

    if (!state.enemy.alive)
        return 0;

    if (!state.player.alive)
        return 1;

    return -1;
}

int combatLogLine(combatstate &state, const char *text)
{
    if (state.logCount >= 16)
        return 0;

    strncpy(state.log[state.logCount], text ? text : "",
            sizeof(state.log[0]) - 1);
    state.log[state.logCount][sizeof(state.log[0]) - 1] = '\0';

    state.logCount++;
    return 1;
}

void showCombatState(const combatstate &state)
{
    showCombatHUD(state.player.name, state.player.hp, state.player.maxHp,
                  state.enemy.name, state.enemy.hp, state.enemy.maxHp);

    cout << "  Round " << state.round
         << "   Phase " << (int)state.phase << endl;
    cout << endl;

    for (int i = 0; i < state.logCount; ++i)
        cout << "    " << state.log[i] << endl;

    cout << endl;
}

/* ---------------- the whole fight ---------------- */

int runCombatToCompletion(combatstate &state)
{
    if (state.phase == CMB_IDLE)
        return -1;

    int guard = 0;

    while (!combatIsOver(state) && guard < 512)
    {
        advanceCombatPhase(state);
        state.ticksElapsed++;

        if (state.phase == CMB_RESOLVE)
        {
            if (state.turn == 0)
                performTurn(state);
            else
                performEnemyTurn(state);
        }

        tickCombatEffects(state);
        guard++;
    }

    int winner = combatWinner(state);

    if (winner == 0)
    {
        targetScore.kills++;
        targetScore.score += 100;
        audioPlaySFX(SFX_VICTORY);
        luaFireEvent(SBW_EVENT_ENEMY_KILLED, 0, 0);
    }
    else
    {
        audioPlaySFX(SFX_DEFEAT);
    }

    g_playerSideWon = (winner == 0);
    state.phase = (winner == 0) ? CMB_VICTORY : CMB_DEFEAT;

    return winner;
}

/* ---------------- bridge ---------------- */

// Build a stat block from the legacy `player` struct.
statblock buildPlayerStatBlock()
{
    statblock stats;
    clearStatBlock(stats);

    stats.attributes.str = charactership.power / 4;
    stats.attributes.dex = charactership.level * 2;
    stats.attributes.con = charactership.sif / 4;
    stats.attributes.vit = charactership.health / 4;
    stats.attributes.intel = charactership.power / 8;
    stats.attributes.wis = charactership.level;
    stats.attributes.spi = charactership.level;
    stats.attributes.lck = charactership.level / 2;
    stats.attributes.wil = charactership.level;
    stats.attributes.cha = charactership.level;

    deriveSubAttributes(stats);
    deriveCombatStats(stats);
    applyStatCaps(stats, isPvPEnabled() ? CTX_PVP : CTX_PVE);

    return stats;
}

// Build a stat block from a fleet entry.
statblock buildEnemyStatBlock(int enemyIndex)
{
    statblock stats;
    clearStatBlock(stats);

    if (enemyIndex < 0 || enemyIndex >= 17)
        return stats;

    const enemyships &enemy = enemyshipsArray[enemyIndex];

    stats.attributes.str = enemy.power / 4;
    stats.attributes.dex = 5;
    stats.attributes.con = enemy.sif / 4;
    stats.attributes.vit = enemy.sp / 4;
    stats.attributes.wil = 5;

    deriveSubAttributes(stats);
    deriveCombatStats(stats);
    applyStatCaps(stats, isPvPEnabled() ? CTX_PVP : CTX_PVE);

    return stats;
}
