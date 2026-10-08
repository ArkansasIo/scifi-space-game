// include/engine/battle_engine.h -- the battle engine.
//
// The combat engine resolves a single fight; the battle engine sequences
// fights into an encounter: waves, patrols, boss phases and rewards.

#ifndef SBW_ENGINE_BATTLE_ENGINE_H
#define SBW_ENGINE_BATTLE_ENGINE_H

enum battlestate
{
    BAT_IDLE = 0,
    BAT_INTRO,
    BAT_WAVE,
    BAT_FIGHT,
    BAT_REWARD,
    BAT_BOSS_PHASE,
    BAT_COMPLETE,
    BAT_LOST,
    BAT_STATE_COUNT
};

struct battleruntime
{
    battlestate state;
    int encounterIndex;
    int wave;
    int waveCount;
    int enemiesRemaining;
    int ticksInState;
    int rewardsGranted;
    int bossPhaseIndex;
    int scoreAwarded;
};

/* ---------------- lifecycle ---------------- */

int battleEngineInit();
int battleEngineStart();
int battleEngineUpdate(int ticks);
void battleEngineStop();
void battleEngineShutdown();

/* ---------------- sequencing ---------------- */

void clearBattleRuntime(battleruntime &rt);

// Begin an encounter from a template index.
void beginBattle(battleruntime &rt, int encounterIndex, int playerLevel,
                 int partySize);

// Advance the state machine one tick.
void tickBattle(battleruntime &rt);

// Are all waves cleared?
int battleComplete(const battleruntime &rt);

// Current wave descriptor text.
const char *battleStateName(battlestate state);

/* ---------------- rewards ---------------- */

// Grant score, XP and loot for clearing a wave.
int grantWaveRewards(battleruntime &rt);

// Grant the end-of-encounter rewards (difficulty, clear time, no-death).
int grantFinalRewards(battleruntime &rt, int clearTimePercent, int noDeath);

/* ---------------- boss phases ---------------- */

// Enter the next boss phase and announce it.
int advanceBossPhase(battleruntime &rt, const bossdata &boss, int hpPercent);

// Apply soft-enrage damage growth for the elapsed fight.
int applySoftEnrage(battleruntime &rt, const bossdata &boss);

// True if the hard enrage time has been reached.
int hardEnrageReached(const battleruntime &rt, const bossdata &boss);

#endif /* SBW_ENGINE_BATTLE_ENGINE_H */
