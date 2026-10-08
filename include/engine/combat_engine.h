// include/engine/combat_engine.h -- the combat engine.
//
// Owns the combat state machine, the turn pipeline and the resolution of
// every action.  See docs/GAMELOGIC.md and docs/FRAMEWORK.md s.15.

#ifndef SBW_ENGINE_COMBAT_ENGINE_H
#define SBW_ENGINE_COMBAT_ENGINE_H

enum combatphase
{
    CMB_IDLE = 0,
    CMB_SETUP,
    CMB_TURN_START,
    CMB_SELECT_ACTION,
    CMB_RESOLVE,
    CMB_TURN_END,
    CMB_VICTORY,
    CMB_DEFEAT,
    CMB_FLED,
    CMB_PHASE_COUNT
};

// One side in a fight.
struct combatant
{
    char name[30];
    statblock stats;
    int hp;
    int maxHp;
    int shield;
    int team; // 0 = player side, 1 = enemy side
    int alive;
    int initiative;
    int threat;
    activeeffect effects[16];
};

// The running fight.
struct combatstate
{
    combatphase phase;
    combatant player;
    combatant enemy;
    int round;
    int turn;
    int pvp; // 0 = PvE rules, 1 = PvP rules
    int totalDamageDealt;
    int totalDamageTaken;
    int ticksElapsed;
    char log[16][MAXLEN];
    int logCount;
};

/* ---------------- lifecycle ---------------- */

int combatEngineInit();
int combatEngineStart();
int combatEngineUpdate(int ticks);
void combatEngineStop();
void combatEngineShutdown();

/* ---------------- state ---------------- */

void clearCombatState(combatstate &state);

// Build a fight from the player's ship and an enemy roster index.
void beginCombat(combatstate &state, int enemyIndex, int pvp);

// Run the whole fight to completion (used by the legacy battle path).
int runCombatToCompletion(combatstate &state);

/* ---------------- turn pipeline ---------------- */

// Initiative = Speed + TacticalSkill + EquipmentBonus
int rollInitiative(const combatant &who);

// Advance to the next phase.  Returns the new phase.
combatphase advanceCombatPhase(combatstate &state);

// Have the current side act.  Returns damage dealt.
int performTurn(combatstate &state);

// Enemy side: score and pick an action, then resolve it.
int performEnemyTurn(combatstate &state);

/* ---------------- actions ---------------- */

enum combataction
{
    ACT_ATTACK = 0,
    ACT_ABILITY,
    ACT_DEFEND,
    ACT_ITEM,
    ACT_FLEE,
    ACT_WAIT
};

int resolveAction(combatstate &state, int actorSide, combataction action);

/* ---------------- effects ---------------- */

// Tick every effect on both combatants, applying DoTs and expiry.
void tickCombatEffects(combatstate &state);

// Apply a buff/debuff by definition index to a side.
int applyCombatEffect(combatstate &state, int side, int definitionIndex);

/* ---------------- queries ---------------- */

int combatIsOver(const combatstate &state);
int combatWinner(const combatstate &state);
int combatLogLine(combatstate &state, const char *text);
void showCombatState(const combatstate &state);

/* ---------------- bridge ---------------- */

// The legacy bridge: fight the old `player` against `enemyshipsArray[index]`.
int runLegacyBattle(int enemyIndex);

#endif /* SBW_ENGINE_COMBAT_ENGINE_H */
