// include/game/ai.h -- enemy AI: state machine + utility-based selection.
// See docs/FRAMEWORK.md section 14.

#ifndef SBW_GAME_AI_H
#define SBW_GAME_AI_H

// Runtime AI state for one enemy.
struct aibrain
{
    int archetypeIndex;
    aiState state;
    int targetId;
    int ticksInState;
    int lastAbility[8];
    int aggroRadius;
    int preferredRange;
    int updateCount;
};

// One ability candidate, scored by the utility system.
struct abilitycandidate
{
    int abilityId;
    int distanceFit;
    int targetHp;
    int cooldownReady;
    int phaseAllowed;
    int playerClustering;
    int threatTarget;
    int totalScore;
    int lastUseTick;
};

// Set up a brain from an archetype.
void initAIBrain(aibrain &brain, const enemyarchetype &archetype);

// Advance the universal state machine one tick.
void tickAIState(aibrain &brain, int distanceToTarget, int targetHpPercent,
                 int alliesAlive);

// Score one ability.  Higher is better.
void scoreAbility(abilitycandidate &ability, int distance, int preferredRange,
                  int targetHpPercent, int cooldownReady, int phaseAllowed,
                  int playerCluster, int threatIsTank, int currentTick);

// Pick the highest-scoring ability, skipping anything on no-repeat lockout.
int pickAbility(abilitycandidate candidates[], int count, int currentTick,
                int lockoutTicks);

// Note that an ability was used, refreshing its no-repeat lockout.
void markAbilityUsed(aibrain &brain, int abilityId, int currentTick);

// True if the ability is still locked out.
int abilityLockedOut(const aibrain &brain, int abilityId, int currentTick);

#endif /* SBW_GAME_AI_H */
