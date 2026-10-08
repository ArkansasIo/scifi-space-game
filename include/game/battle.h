// include/game/battle.h -- turn based battle generator.
#ifndef SBW_GAME_BATTLE_H
#define SBW_GAME_BATTLE_H

void initializeBattle();

// Run a fight against a specific fleet entry through the combat engine.
// Returns 0 for a player win, 1 for an enemy win, -1 if unresolved.
int runLegacyBattle(int enemyIndex);

#endif
