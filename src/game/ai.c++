// src/game/ai.c++ -- enemy AI helpers: state machine and utility scoring.
// See docs/FRAMEWORK.md s.14.
//
// The pure rules live here; src/engine/ai_engine.c++ drives them over time.

#include "spacebattlerpg.h"

void initAIBrain(aibrain &brain, const enemyarchetype &archetype)
{
    memset(&brain, 0, sizeof(brain));

    brain.archetypeIndex = findArchetype(archetype.role, archetype.tier);
    brain.state = AI_IDLE;
    brain.targetId = -1;
    brain.ticksInState = 0;
    brain.aggroRadius = archetype.aggroRadius;
    brain.preferredRange = archetype.preferredRange;
    brain.updateCount = 0;

    aiResetLockouts(brain);
}

void tickAIState(aibrain &brain, int distanceToTarget, int targetHpPercent,
                 int alliesAlive)
{
    (void)targetHpPercent;

    brain.ticksInState++;

    switch (brain.state)
    {
    case AI_IDLE:
        if (distanceToTarget <= brain.aggroRadius)
        {
            brain.state = AI_ALERT;
            brain.ticksInState = 0;
        }
        break;

    case AI_PATROL:
        if (distanceToTarget <= brain.aggroRadius)
        {
            brain.state = AI_ALERT;
            brain.ticksInState = 0;
        }
        break;

    case AI_ALERT:
        brain.state = AI_ENGAGE;
        brain.ticksInState = 0;
        break;

    case AI_ENGAGE:
        brain.state = AI_COMBAT;
        brain.ticksInState = 0;
        break;

    case AI_COMBAT:
        // Break off when there is nothing left to support the fight.
        if (alliesAlive <= 0 && brain.ticksInState > 20)
        {
            brain.state = AI_RETREAT;
            brain.ticksInState = 0;
        }
        break;

    case AI_RETREAT:
        if (distanceToTarget > brain.aggroRadius)
        {
            brain.state = AI_RESET;
            brain.ticksInState = 0;
        }
        break;

    case AI_RESET:
        brain.targetId = -1;
        brain.state = AI_IDLE;
        brain.ticksInState = 0;
        break;

    default:
        break;
    }
}

void scoreAbility(abilitycandidate &ability, int distance, int preferredRange,
                  int targetHpPercent, int cooldownReady, int phaseAllowed,
                  int playerCluster, int threatIsTank, int currentTick)
{
    // 1. Distance fit: how close the situation is to the ideal range.
    int gap = distance - preferredRange;
    if (gap < 0)
        gap = -gap;

    ability.distanceFit = 100 - gap * 10;
    if (ability.distanceFit < 0)
        ability.distanceFit = 0;

    // 2. Target HP: low HP is an execute window.
    ability.targetHp = (targetHpPercent < 30) ? 60 : 20;

    // 3. Cooldown and phase gating.
    ability.cooldownReady = cooldownReady ? 40 : 0;
    ability.phaseAllowed = phaseAllowed ? 30 : 0;

    // 4. Player clustering favours area abilities.
    ability.playerClustering = (playerCluster >= 3) ? 35 : 10;

    // 5. Threat target: the tank is the expected target.
    ability.threatTarget = threatIsTank ? 25 : 15;

    // 6. A bonus for abilities that have not been used recently.
    int since = currentTick - ability.lastUseTick;

    if (since < 0)
        since = 0;

    if (since > 100)
        since = 100;

    ability.totalScore = ability.distanceFit + ability.targetHp + ability.cooldownReady + ability.phaseAllowed + ability.playerClustering + ability.threatTarget + since;
}

int pickAbility(abilitycandidate candidates[], int count, int currentTick,
                int lockoutTicks)
{
    if (!candidates || count <= 0)
        return -1;

    int best = -1;
    int bestScore = -1;

    for (int i = 0; i < count; ++i)
    {
        if (currentTick - candidates[i].lastUseTick < lockoutTicks)
            continue;

        if (candidates[i].totalScore > bestScore)
        {
            bestScore = candidates[i].totalScore;
            best = candidates[i].abilityId;
        }
    }

    return best;
}

void markAbilityUsed(aibrain &brain, int abilityId, int currentTick)
{
    if (abilityId < 0 || abilityId >= 8)
        return;

    brain.lastAbility[abilityId] = currentTick;
}

int abilityLockedOut(const aibrain &brain, int abilityId, int currentTick)
{
    if (abilityId < 0 || abilityId >= 8)
        return 0;

    // A short no-repeat lockout so the AI cannot spam one ability.
    return (currentTick - brain.lastAbility[abilityId]) < 3;
}
