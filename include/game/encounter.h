// include/game/encounter.h -- encounter generation from templates.
// See docs/FRAMEWORK.md sections 7, 8 and 13.

#ifndef SBW_GAME_ENCOUNTER_H
#define SBW_GAME_ENCOUNTER_H

// One spawned enemy within an encounter.
struct encounterunit
{
    int archetypeIndex;
    int tier;
    int role;
    int affixIndex;
    int scaledHP;
    int scaledDamage;
    int spawnSlot;
};

// A generated encounter ready to be fought.
struct encounterinstance
{
    char name[30];
    encounterkind kind;
    int unitCount;
    encounterunit units[MAX_ARCHETYPES];
    int bossIndex;
    int waveCount;
    int timed;
    int timeLimit;
    int powerBudget;
};

// Zero an encounter instance.
void clearEncounter(encounterinstance &enc);

// Build an encounter from a template index, scaling to the given party.
void buildEncounter(encounterinstance &enc, int templateIndex, int playerLevel,
                    int partySize, enemytier tier);

// Build a random open-world pack (3 trash + 1 caster + patrol).
void buildOpenWorldPack(encounterinstance &enc, int playerLevel);

// Build a dungeon room: pack, pack+miniboss, puzzle or gauntlet.
void buildDungeonRoom(encounterinstance &enc, int playerLevel, int roomSeed);

// Build a raid encounter: boss + adds, arena morph or council fight.
void buildRaidEncounter(encounterinstance &enc, int bossIndex, int partySize);

// Pick a boss phase for the boss's current HP percentage.
const bossphase *currentBossPhase(const bossdata &boss, int hpPercent);

// True when the boss should hard-enrage at this elapsed time.
int bossShouldEnrage(const bossdata &boss, int elapsedTicks);

// Soft-enrage damage bonus: +softEnrageRate% per 15 seconds elapsed.
int softEnrageBonus(const bossdata &boss, int elapsedTicks);

#endif /* SBW_GAME_ENCOUNTER_H */
