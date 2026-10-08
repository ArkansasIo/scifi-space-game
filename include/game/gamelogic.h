// include/game/gamelogic.h -- top-level game logic and feature entry points.
//
// This is the single header that ties the framework together: it exposes the
// "game menu" helpers that main.c++ calls, and documents the feature set.
//
// See docs/FRAMEWORK.md for the full design and docs/GAMELOGIC.md for the
// function-by-function breakdown.

#ifndef SBW_GAME_GAMELOGIC_H
#define SBW_GAME_GAMELOGIC_H

// ---- lifecycle ----

// Bring the whole framework up: label arrays, sub-arrays, every data table
// and the galaxy map.  Safe to call more than once.
void initializeFramework();

// Tear down and reset live state (effects, threat, encounters).
void resetFrameworkState();

// ---- feature entry points ----

// Print a character sheet from a stat block.
void showStatSheet(const statblock &stats);

// Print the player's current ship / character summary.
void showPlayerSummary();

// Print the active effect list for a target.
void showActiveEffects(const activeeffect list[], int count);

// Print a full item tooltip, showing the PvE and PvP tables side by side.
void showItemTooltip(int itemIndex);

// Print a generated encounter roster.
void showEncounter(const encounterinstance &enc);

// Print the boss roster and their phase plans.
void showBossRoster();

// Print the galaxy map (sectors, systems, planets).
void showGalaxyMap();

// Print the empire dashboard (attributes, resources, rank).
void showEmpireDashboard();

// ---- feature toggles / queries ----

// Enable or disable PvP rules globally (affects every combatcontext).
void setPvPEnabled(int enabled);
int isPvPEnabled();

// Difficulty tier currently selected (0 = normal).
int currentDifficultyTier();

// One-line description of a framework feature, by id.
const char *featureDescription(int featureId);

// Total number of documented features.
int featureCount();

#endif /* SBW_GAME_GAMELOGIC_H */
