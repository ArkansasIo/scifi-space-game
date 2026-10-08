// include/game/threat.h -- aggro / threat model.
// See docs/FRAMEWORK.md section 9.

#ifndef SBW_GAME_THREAT_H
#define SBW_GAME_THREAT_H

// One entry in a threat table (one combatant).
struct threatentry
{
    int entityId;
    int threat;
    int forcedUntil;
};

// Zero a threat table.
void clearThreatTable(threatentry table[], int count);

// Find the index of an entity, or -1.
int findThreatIndex(const threatentry table[], int count, int entityId);

// Threat = Damage*D + Healing*H + FlatThreat + TauntOverride
int computeThreat(int damage, int healing, int flatThreat, int dmgWeight,
                  int healWeight);

// Add generated threat, applying the stance multiplier (percent).
int addThreat(threatentry table[], int count, int entityId, int amount,
              int stancePercent);

// Register a taunt: forces target for N ticks and sets threat to top + 1.
int tauntThreat(threatentry table[], int count, int entityId, int ticks);

// Index of the highest-threat living combatant, or -1.
int highestThreatIndex(const threatentry table[], int count);

// Highest-threat combatant not currently taunting someone else.
int pickTarget(const threatentry table[], int count);

// Decay a percentage of all threat.
void decayThreat(threatentry table[], int count, int percent);

// Advance forced-target timers one tick.
void tickThreat(threatentry table[], int count);

#endif /* SBW_GAME_THREAT_H */
