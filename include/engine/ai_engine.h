// include/engine/ai_engine.h -- the AI engine.
//
// Drives every enemy brain: the universal state machine, combat sub-states
// and utility-based ability selection.  See docs/FRAMEWORK.md s.14.

#ifndef SBW_ENGINE_AI_ENGINE_H
#define SBW_ENGINE_AI_ENGINE_H

// Combat sub-state, entered once the brain is in AI_COMBAT.
enum aicombsubstate
{
    SUB_MAINTAIN_RANGE = 0,
    SUB_CLOSE_DISTANCE,
    SUB_BURST_WINDOW,
    SUB_DEFENSIVE_WINDOW,
    SUB_SUMMON_ADDS,
    SUB_CAST_ULTIMATE,
    SUB_PHASE_TRANSITION,
    SUB_COUNT
};

struct aiworld
{
    int currentTick;
    int playerHpPercent;
    int playerClusterSize;
    int alliesAlive;
    int tankThreat;
    int healerThreat;
    int distanceToTarget;
};

/* ---------------- lifecycle ---------------- */

int aiEngineInit();
int aiEngineStart();
int aiEngineUpdate(int ticks);
void aiEngineStop();
void aiEngineShutdown();

/* ---------------- brains ---------------- */

void clearAIWorld(aiworld &world);

// Update one brain: state machine, then sub-state, then ability pick.
void aiUpdateBrain(aibrain &brain, const aiworld &world);

// The universal state machine.
aiState aiAdvanceState(aibrain &brain, const aiworld &world);

// Choose a combat sub-state for the current situation.
aicombsubstate aiChooseSubState(const aibrain &brain, const aiworld &world);

// Score every ability a brain could use and return the best id.
int aiSelectAbility(aibrain &brain, const aiworld &world,
                    abilitycandidate candidates[], int count);

// Build a candidate list from an archetype's kit.
int aiBuildCandidates(const enemyarchetype &archetype,
                      abilitycandidate candidates[], int maxCandidates);

/* ---------------- helpers ---------------- */

const char *aiStateName(aiState state);
const char *aiSubStateName(aicombsubstate sub);
int aiShouldFlee(const aibrain &brain, const aiworld &world);
void aiResetLockouts(aibrain &brain);

#endif /* SBW_ENGINE_AI_ENGINE_H */
