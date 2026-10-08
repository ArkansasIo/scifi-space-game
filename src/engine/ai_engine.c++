// src/engine/ai_engine.c++ -- the AI engine.
//
// Drives every enemy brain through the universal state machine, then the
// combat sub-state selection, then utility-scored ability choice.
// See docs/FRAMEWORK.md s.14.

#include "spacebattlerpg.h"
#include "engine/ai_engine.h"

static int g_aiRunning = 0;

/* ---------------- lifecycle ---------------- */

int aiEngineInit()
{
    return 1;
}

int aiEngineStart()
{
    g_aiRunning = 1;
    return 1;
}

int aiEngineUpdate(int ticks)
{
    (void)ticks;
    return g_aiRunning;
}

void aiEngineStop()
{
    g_aiRunning = 0;
}

void aiEngineShutdown()
{
    g_aiRunning = 0;
}

/* ---------------- world ---------------- */

void clearAIWorld(aiworld &world)
{
    memset(&world, 0, sizeof(world));
    world.playerHpPercent = 100;
    world.alliesAlive = 1;
    world.distanceToTarget = 1;
}

/* ---------------- state machine ---------------- */

aiState aiAdvanceState(aibrain &brain, const aiworld &world)
{
    brain.ticksInState++;

    switch (brain.state)
    {
    case AI_IDLE:
        if (brain.aggroRadius >= world.distanceToTarget)
            brain.state = AI_ALERT;
        break;

    case AI_ALERT:
        brain.state = AI_ENGAGE;
        break;

    case AI_ENGAGE:
        brain.state = AI_COMBAT;
        brain.ticksInState = 0;
        break;

    case AI_COMBAT:
        if (aiShouldFlee(brain, world))
            brain.state = AI_RETREAT;
        break;

    case AI_RETREAT:
        // Retreating enemies reset when they break line of sight.
        if (world.distanceToTarget > brain.aggroRadius)
            brain.state = AI_RESET;
        break;

    case AI_RESET:
        brain.targetId = -1;
        brain.state = AI_IDLE;
        brain.ticksInState = 0;
        break;

    default:
        break;
    }

    return brain.state;
}

aicombsubstate aiChooseSubState(const aibrain &brain, const aiworld &world)
{
    if (brain.state != AI_COMBAT)
        return SUB_MAINTAIN_RANGE;

    // A summoner prefers to call for help before committing.
    if (brain.archetypeIndex >= 0 && brain.archetypeIndex < archetypeCount && archetypeArray[brain.archetypeIndex].role == ROLE_SUMMONER && world.alliesAlive < 2)
        return SUB_SUMMON_ADDS;

    // Low player HP is the signal to burst.
    if (world.playerHpPercent < 25)
        return SUB_BURST_WINDOW;

    // The AI backs off when it is hurt far worse than its target.
    if (aiShouldFlee(brain, world))
        return SUB_DEFENSIVE_WINDOW;

    // Clustered players favour an ultimate; spread players favour repositioning.
    if (world.playerClusterSize >= 3)
        return SUB_CAST_ULTIMATE;

    if (world.distanceToTarget > brain.preferredRange)
        return SUB_CLOSE_DISTANCE;

    return SUB_MAINTAIN_RANGE;
}

int aiShouldFlee(const aibrain &brain, const aiworld &world)
{
    (void)brain;

    // A simple, readable retreat rule: heavily outnumbered and badly hurt.
    if (world.alliesAlive <= 0 && world.playerHpPercent > 60)
        return 1;

    return 0;
}

/* ---------------- ability selection ---------------- */

void aiUpdateBrain(aibrain &brain, const aiworld &world)
{
    brain.updateCount++;

    aiAdvanceState(brain, world);

    if (brain.state != AI_COMBAT)
        return;

    aicombsubstate sub = aiChooseSubState(brain, world);
    (void)sub;

    abilitycandidate candidates[8];
    int count = 0;

    if (brain.archetypeIndex >= 0 && brain.archetypeIndex < archetypeCount)
    {
        count = aiBuildCandidates(archetypeArray[brain.archetypeIndex],
                                  candidates, 8);
    }

    if (count > 0)
    {
        int chosen = aiSelectAbility(brain, world, candidates, count);

        if (chosen >= 0)
            markAbilityUsed(brain, chosen, world.currentTick);
    }
}

int aiBuildCandidates(const enemyarchetype &archetype,
                      abilitycandidate candidates[], int maxCandidates)
{
    int capacity = archetype.abilityCount;

    if (capacity > maxCandidates)
        capacity = maxCandidates;

    if (capacity < 0)
        capacity = 0;

    for (int i = 0; i < capacity; ++i)
    {
        candidates[i].abilityId = i;
        candidates[i].distanceFit = 0;
        candidates[i].targetHp = 0;
        candidates[i].cooldownReady = 0;
        candidates[i].phaseAllowed = 0;
        candidates[i].playerClustering = 0;
        candidates[i].threatTarget = 0;
        candidates[i].totalScore = 0;
        candidates[i].lastUseTick = -999;
    }

    return capacity;
}

int aiSelectAbility(aibrain &brain, const aiworld &world,
                    abilitycandidate candidates[], int count)
{
    int bestIndex = -1;
    int bestScore = -1;

    for (int i = 0; i < count; ++i)
    {
        abilitycandidate &candidate = candidates[i];

        if (abilityLockedOut(brain, candidate.abilityId, world.currentTick))
            continue;

        scoreAbility(candidate,
                     world.distanceToTarget,
                     brain.preferredRange,
                     world.playerHpPercent,
                     1, // cooldown ready
                     1, // phase allowed
                     world.playerClusterSize,
                     (world.tankThreat > world.healerThreat) ? 1 : 0,
                     world.currentTick);

        if (candidate.totalScore > bestScore)
        {
            bestScore = candidate.totalScore;
            bestIndex = candidate.abilityId;
        }
    }

    return bestIndex;
}

/* ---------------- names ---------------- */

// Clear every per-ability no-repeat lockout on a brain.  Called when a brain
// is first created and whenever a phase transition resets the rotation.
void aiResetLockouts(aibrain &brain)
{
    for (int i = 0; i < 8; ++i)
        brain.lastAbility[i] = -999;
}

const char *aiStateName(aiState state)
{
    switch (state)
    {
    case AI_IDLE:
        return "Idle";
    case AI_PATROL:
        return "Patrol";
    case AI_ALERT:
        return "Alert";
    case AI_ENGAGE:
        return "Engage";
    case AI_COMBAT:
        return "Combat";
    case AI_RETREAT:
        return "Retreat";
    case AI_RESET:
        return "Reset";
    default:
        return "Unknown";
    }
}

const char *aiSubStateName(aicombsubstate sub)
{
    switch (sub)
    {
    case SUB_MAINTAIN_RANGE:
        return "Maintain Range";
    case SUB_CLOSE_DISTANCE:
        return "Close Distance";
    case SUB_BURST_WINDOW:
        return "Burst Window";
    case SUB_DEFENSIVE_WINDOW:
        return "Defensive Window";
    case SUB_SUMMON_ADDS:
        return "Summon Adds";
    case SUB_CAST_ULTIMATE:
        return "Cast Ultimate";
    case SUB_PHASE_TRANSITION:
        return "Phase Transition";
    default:
        return "Unknown";
    }
}
